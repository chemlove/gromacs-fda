/*
 * FDAShortestPathTest.cpp
 *
 *  Created on: Feb 13, 2015
 *      Author: Bernd Doser, HITS gGmbH
 */

#include "gromacs/gmxana/gmx_ana.h"
#include "gromacs/options/filenameoption.h"
#include "gromacs/fileio/futil.h"
#include "gromacs/fileio/path.h"
#include "gromacs/gmxpreprocess/grompp.h"
#include "programs/mdrun/mdrun_main.h"
#include "testutils/cmdlinetest.h"
#include "testutils/integrationtests.h"
#include "testutils/TextSplitter.h"
#include "testutils/LogicallyErrorComparer.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct TestDataStructure
{
    TestDataStructure(
        std::string const& testDirectory,
        std::vector<std::string> const& cmdline,
        std::string const& groupname,
        std::string const& result,
        std::string const& reference
    )
     : testDirectory(testDirectory),
       cmdline(cmdline),
       groupname(groupname),
       result(result),
       reference(reference)
    {}

    std::string testDirectory;
    std::vector<std::string> cmdline;
    std::string groupname;
    std::string result;
    std::string reference;
};

} // namespace anonymous

//! Test fixture for FDA
class FDAShortestPathTest : public ::testing::WithParamInterface<TestDataStructure>,
                            public ::gmx::test::IntegrationTestFixture
{};

//! Test body for FDA
TEST_P(FDAShortestPathTest, Basic)
{
    std::string cwd = gmx::Path::getWorkingDirectory();
    std::string dataPath = std::string(fileManager_.getInputDataDirectory()) + "/data";
    std::string testPath = fileManager_.getTemporaryFilePath("/" + GetParam().testDirectory);

    std::string cmd = "mkdir -p " + testPath;
    ASSERT_FALSE(system(cmd.c_str()));

    cmd = "cp -r " + dataPath + "/" + GetParam().testDirectory + "/* " + testPath;
    ASSERT_FALSE(system(cmd.c_str()));

    gmx_chdir(testPath.c_str());

    ::gmx::test::CommandLine caller;
    caller.append("gmx_fda fda_shortest_path");
    for (std::vector<std::string>::const_iterator iterCur(GetParam().cmdline.begin()), iterNext(GetParam().cmdline.begin() + 1),
        iterEnd(GetParam().cmdline.end()); iterCur != iterEnd; ++iterCur, ++iterNext)
    {
        if (iterNext == iterEnd or iterNext->substr(0,1) == "-") caller.append(*iterCur);
        else {
            caller.addOption(iterCur->c_str(), iterNext->c_str());
            ++iterCur, ++iterNext;
        }
    }
    caller.addOption("-o", GetParam().result);

    std::cout << caller.toString() << std::endl;

    if (!GetParam().groupname.empty()) redirectStringToStdin((GetParam().groupname + "\n").c_str());

    ASSERT_FALSE(gmx_fda_shortest_path(caller.argc(), caller.argv()));

    const double error_factor = 1.0e4;
    const bool weight_by_magnitude = false;
    const bool ignore_sign = true;

    LogicallyEqualComparer<weight_by_magnitude,ignore_sign> comparer(error_factor);

    // compare atom pairs
    EXPECT_TRUE((compare(TextSplitter(GetParam().reference), TextSplitter(GetParam().result), comparer)));

    gmx_chdir(cwd.c_str());
}

INSTANTIATE_TEST_CASE_P(AllFDAShortestPathTests, FDAShortestPathTest, ::testing::Values(
    TestDataStructure(
        "glycine_trimer",
        {"-ipf", "fda.pfr", "-s", "glycine_trimer.pdb", "-n", "index.ndx", "-frame", "0", "-source", "0", "-dest", "2", "-nk", "2"},
        "C-alpha",
        "result.pdb",
        "ref0.pdb"
    ),
    TestDataStructure(
        "glycine_trimer",
        {"-ipf", "fda.pfr", "-s", "glycine_trimer.pdb", "-n", "index.ndx", "-frame", "average 11", "-source", "0", "-dest", "2", "-nk", "2"},
        "C-alpha",
        "result.pdb",
        "ref1.pdb"
    ),
    TestDataStructure(
        "glycine_trimer",
        {"-ipf", "fda.pfr", "-s", "glycine_trimer.pdb", "-n", "index.ndx", "-frame", "average 11", "-source", "0", "-dest", "2", "-nk", "2", "-convert"},
        "C-alpha",
        "result.pdb",
        "ref2.pdb"
    ),
    TestDataStructure(
        "glycine_trimer",
        {"-ipf", "fda.pfr", "-s", "glycine_trimer.pdb", "-n", "index.ndx", "-frame", "all", "-source", "0", "-dest", "2", "-nk", "2"},
        "C-alpha",
        "result.pdb",
        "ref3.pdb"
    )
));
