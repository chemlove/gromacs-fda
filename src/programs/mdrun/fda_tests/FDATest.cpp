/*
 * FDATest.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Bernd Doser, HITS gGmbH <bernd.doser@h-its.org>
 */

#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "gromacs/simd/support.h"
#include "gromacs/fda/PairwiseForces.h"
#include "gromacs/options/filenameoption.h"
#include "gromacs/utility/futil.h"
#include "gromacs/utility/path.h"
#include "gromacs/utility/real.h"
#include "gromacs/gmxpreprocess/grompp.h"
#include "programs/mdrun/mdrun_main.h"
#include "testutils/cmdlinetest.h"
#include "testutils/testfilemanager.h"
#include "testutils/TextSplitter.h"
#include "testutils/LogicallyErrorComparer.h"

namespace gmx
{
namespace test
{
namespace
{

struct TestDataStructure
{
    TestDataStructure(
        std::string const& testDirectory,
        std::string const& atomFileExtension,
        std::string const& residueFileExtension,
        std::string const& trajectoryFilename = "traj.trr",
        bool is_vector = false,
        bool must_die = false
    )
      : testDirectory(testDirectory),
        atomFileExtension(atomFileExtension),
        residueFileExtension(residueFileExtension),
        trajectoryFilename(trajectoryFilename),
        is_vector(is_vector),
        must_die(must_die)
    {}

    std::string testDirectory;
    std::string atomFileExtension;
    std::string residueFileExtension;
    std::string trajectoryFilename;
    bool is_vector;
    bool must_die;
};

//! Test fixture for FDA
class FDATest : public ::testing::WithParamInterface<TestDataStructure>,
                public CommandLineTestBase
{};

//! Test body for FDA
TEST_P(FDATest, Basic)
{
    std::cout << GetParam().testDirectory << std::endl;

    std::string cwd = gmx::Path::getWorkingDirectory();
    std::string dataPath = std::string(fileManager().getInputDataDirectory()) + "/data";
    std::string testPath = fileManager().getTemporaryFilePath("/" + GetParam().testDirectory);

    std::string cmd = "mkdir -p " + testPath;
    ASSERT_FALSE(system(cmd.c_str()));

    cmd = "cp -r " + dataPath + "/" + GetParam().testDirectory + "/* " + testPath;
    ASSERT_FALSE(system(cmd.c_str()));

    gmx_chdir(testPath.c_str());

    std::string atomFilename = "fda." + GetParam().atomFileExtension;
    std::string atomOption = "-" + GetParam().atomFileExtension;
    std::string atomReference = atomFilename + ".ref";
    std::string residueFilename = "fda." + GetParam().residueFileExtension;
    std::string residueOption = "-" + GetParam().residueFileExtension;
    std::string residueReference = residueFilename + ".ref";

    ::gmx::test::CommandLine callRerun;
    callRerun.append("gmx_fda mdrun");
    callRerun.addOption("-deffnm", "rerun");
    callRerun.addOption("-s", "topol.tpr");
    callRerun.addOption("-rerun", GetParam().trajectoryFilename);
    callRerun.addOption("-nt", "1");
    callRerun.addOption("-pfn", "index.ndx");
    callRerun.addOption("-pfi", "fda.pfi");
    if (!GetParam().atomFileExtension.empty()) callRerun.addOption(atomOption.c_str(), atomFilename.c_str());
    if (!GetParam().residueFileExtension.empty()) callRerun.addOption(residueOption.c_str(), residueFilename.c_str());

    std::cout << "command: " << callRerun.toString() << std::endl;

    if (GetParam().must_die) {
        EXPECT_EXIT(gmx_mdrun(callRerun.argc(), callRerun.argv()), ::testing::ExitedWithCode(1), "");
    } else {
        ASSERT_FALSE(gmx_mdrun(callRerun.argc(), callRerun.argv()));

		const double error_factor = 1e4;
		const bool weight_by_magnitude = true;
		const bool ignore_sign = false;

		LogicallyEqualComparer<weight_by_magnitude, ignore_sign> comparer(error_factor);

		// Check results
		if (!GetParam().atomFileExtension.empty()) {
			if (GetParam().atomFileExtension == "pfa")
				if (GetParam().is_vector)
					EXPECT_TRUE((fda::PairwiseForces<fda::Force<fda::Vector>>(atomFilename).equal(
						fda::PairwiseForces<fda::Force<fda::Vector>>(atomReference), comparer)));
				else
					EXPECT_TRUE((fda::PairwiseForces<fda::Force<real>>(atomFilename).equal(
						fda::PairwiseForces<fda::Force<real>>(atomReference), comparer)));
			else
				EXPECT_TRUE((equal(TextSplitter(atomFilename), TextSplitter(atomReference), comparer)));
		}
		if (!GetParam().residueFileExtension.empty()) {
			if (GetParam().residueFileExtension == "pfr")
				if (GetParam().is_vector)
					EXPECT_TRUE((fda::PairwiseForces<fda::Force<fda::Vector>>(residueFilename).equal(
						fda::PairwiseForces<fda::Force<fda::Vector>>(residueReference), comparer)));
				else
					EXPECT_TRUE((fda::PairwiseForces<fda::Force<real>>(residueFilename).equal(
						fda::PairwiseForces<fda::Force<real>>(residueReference), comparer)));
			else
				EXPECT_TRUE((equal(TextSplitter(residueFilename), TextSplitter(residueReference), comparer)));
		}
		gmx_chdir(cwd.c_str());
    }
}

std::vector<TestDataStructure> get_tests()
{
	std::vector<TestDataStructure> tests {
		{"alagly_verlet_summed_scalar", "pfa", "pfr"},
		{"alagly_verlet_pbc_summed_scalar", "pfa", "pfr"},
		{"alagly_verlet_pbc_summed_scalar_binary", "pfa", "pfr", "traj.trr", false, false},
		{"cmap_ignore_missing_potentials", "", "psr", "traj.xtc"},
		{"cmap", "", "psr", "traj.xtc", false, true}
	};

	if (simdCompiled() == SimdType::None) {
		tests.push_back({"alagly_pairwise_forces_scalar", "pfa", "pfr"});
		tests.push_back({"alagly_pairwise_forces_scalar_atom_based", "pfa", ""});
		tests.push_back({"alagly_pairwise_forces_scalar_no_residue_based", "pfa", ""});
		tests.push_back({"alagly_pairwise_forces_scalar_detailed_no_residue_based", "pfa", ""});
		tests.push_back({"alagly_pairwise_forces_vector", "pfa", "pfr", "traj.trr", true});
		tests.push_back({"alagly_punctual_stress", "psa", "psr"});
		tests.push_back({"alagly_punctual_stress_normalized", "psa", "psr"});
		tests.push_back({"alagly_punctual_stress_normalized_renumbered", "psa", "psr"});
		tests.push_back({"alagly_pairwise_forces_scalar_detailed_nonbonded", "pfa", "pfr"});
		tests.push_back({"alagly_pairwise_forces_vector_detailed_nonbonded", "pfa", "pfr", "traj.trr", true});
		tests.push_back({"alagly_group_excl", "pfa", "pfr"});
		tests.push_back({"alagly_group_excl_uncomplete_cgs", "pfa", "pfr"});
		tests.push_back({"alagly_pairwise_forces_scalar_all", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl1", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl2", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl3", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl4", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl5", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_excl6", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_group_bonded_excl1", "pfa", "pfr"});
		tests.push_back({"glycine_trimer_virial_stress", "vsa", ""});
		tests.push_back({"glycine_trimer_virial_stress_von_mises", "vma", ""});
		tests.push_back({"alagly_deprecated_keywords", "pfa", "pfr", "", false, true});
		tests.push_back({"alagly_unknown_option", "pfa", "pfr", "", false, true});
		tests.push_back({"vwf_a2_domain_nframes1_pairwise_forces_scalar", "pfa", "pfr", "traj.xtc"});
		tests.push_back({"vwf_a2_domain_nframes1_punctual_stress", "psa", "psr", "traj.xtc"});
		tests.push_back({"vwf_a2_domain_nframes10_pairwise_forces_scalar", "pfa", "pfr", "traj.xtc"});
		tests.push_back({"vwf_a2_domain_nframes10_punctual_stress", "psa", "psr", "traj.xtc"});
	}

	return tests;
}

INSTANTIATE_TEST_CASE_P(AllFDATests, FDATest, ::testing::ValuesIn(get_tests()));

} // namespace
} // namespace test
} // namespace gmx
