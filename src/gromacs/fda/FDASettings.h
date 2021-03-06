/*
 * FDASettings.h
 *
 *  Created on: Nov 9, 2016
 *      Author: Bernd Doser, HITS gGmbH <bernd.doser@h-its.org>
 */

#ifndef SRC_GROMACS_FDA_FDASETTINGS_H_
#define SRC_GROMACS_FDA_FDASETTINGS_H_

#include <map>
#include <vector>
#include "gromacs/commandline/filenm.h"
#include "gromacs/topology/topology.h"
#include "InteractionType.h"
#include "OnePair.h"
#include "ResidueRenumber.h"
#include "ResultType.h"
#include "Vector2Scalar.h"

namespace fda {

/// Settings
struct FDASettings
{
    /// Default constructor
    FDASettings()
     : atom_based_result_type(ResultType::NO),
       residue_based_result_type(ResultType::NO),
	   binary_result_file(false),
       one_pair(OnePair::DETAILED),
       v2s(Vector2Scalar::NORM),
       residues_renumber(ResiduesRenumber::AUTO),
       no_end_zeros(false),
       syslen_atoms(0),
       syslen_residues(0),
       time_averaging_period(1),
       type(InteractionType_NONE),
       nonbonded_exclusion_on(true),
       bonded_exclusion_on(true),
       index_group1(-1),
       index_group2(-1),
       groups(nullptr),
       groupnames(nullptr),
	   threshold(1e-10)
    {}

    /// Construction by input file
    FDASettings(int nfile, const t_filenm fnm[], gmx_mtop_t *mtop, bool parallel_execution);

    /// Returns true if atom i is in fda groups
    bool atom_in_groups(int i) const {
        return (sys_in_group1[i] or sys_in_group2[i]);
    }

    /// Returns true if atoms i and j are in fda groups
    bool atoms_in_groups(int i, int j) const {
        return ((sys_in_group1[i] and sys_in_group2[j]) or (sys_in_group1[j] and sys_in_group2[i]));
    }

    /// Makes a list of residue numbers based on atom numbers of this group.
    /// This is slightly more complex than needed to allow the residue numbers to retain the ordering given to atoms.
    std::vector<int> groupatoms2residues(std::vector<int> const& group_atoms) const;

    /// Fill in the map between atom and residue index
    /// Adapted from the code fragment in Data_Structures page on GROMACS website
    void fill_atom2residue(gmx_mtop_t *mtop);

    /// Returns the global residue number - based on mtop_util.c::gmx_mtop_atominfo_global(),
    /// equivalent to a call to it with mtop->maxres_renum = INT_MAX
    int get_global_residue_number(gmx_mtop_t *mtop, int atnr_global) const;

    int get_atom2residue(int i) const { return atom_2_residue[i]; }

    bool compatibility_mode(ResultType const& r) const {
        return r == ResultType::COMPAT_BIN or r == ResultType::COMPAT_ASCII;
    }

    bool stress_mode(ResultType const& r) const {
        return r == ResultType::PUNCTUAL_STRESS or r == ResultType::VIRIAL_STRESS or r == ResultType::VIRIAL_STRESS_VON_MISES;
    }

    bool PF_or_PS_mode(ResultType const& r) const {
        return r == ResultType::PAIRWISE_FORCES_VECTOR or r == ResultType::PAIRWISE_FORCES_SCALAR or r == ResultType::PUNCTUAL_STRESS;
    }

    bool VS_mode(ResultType const& r) const {
        return r == ResultType::VIRIAL_STRESS or r == ResultType::VIRIAL_STRESS_VON_MISES;
    }

    /// ResultType for atom based forces
    ResultType atom_based_result_type;

    /// ResultType for residue based forces
    ResultType residue_based_result_type;

    /// If true, the result files will be written in binary format
    bool binary_result_file;

    /// OnePair defines the way the interactions between the same pair of atoms are stored
    OnePair one_pair;

    /// Define conversion from vector to scalar
    Vector2Scalar v2s;

    /// detect/force residue renumbering
    ResiduesRenumber residues_renumber;

    /// If True, trim the line such that the zeros at the end are not written.
    /// if False (default), all per atom/residue data is written.
    bool no_end_zeros;

    /// Total number of atoms in the system.
    /// This is a local copy to avoid passing too many variables down the function call stack
    int syslen_atoms;

    /// Maximum of residue nr. + 1; residue nr. doesn't have to be continuous, there can be gaps
    int syslen_residues;

    /// Mapping of real atom number to index in the pf array
    std::map<int, int> sys2pf_atoms;

    /// Mapping of real residue number to index in the pf array
    std::map<int, int> sys2pf_residues;

    /// Number of steps to average before writing.
    /// If 1 (default), no averaging is done.
    /// If 0 averaging is done over all steps so only one frame is written at the end.
    int time_averaging_period;

    /// Output file name for atoms if AtomBased is non-zero
    std::string atom_based_result_filename;

    /// Output file name for residues if ResidueBased is non-zero
    std::string residue_based_result_filename;

    /// If 0 if atom not in group1, if 1 if atom in group1, length of syslen_atoms
    std::vector<char> sys_in_group1;

    /// If 0 if atom not in group2, if 1 if atom in group2, length of syslen_atoms
    std::vector<char> sys_in_group2;

    /// Name of group for output in compatibility mode
    std::string groupname;

    /// Interaction types that are interesting, set based on input file; functions are supposed to test against this before calculating/storing data
    InteractionType type;

    /// Stores the residue number for each atom; array of length syslen; only initialized if ResidueBased is non-zero
    std::vector<int> atom_2_residue;

    /// Stores the number of atoms for each residue; only initialized if ResidueBased is non-zero
    std::vector<int> residue_size;

    /// Version of force matrix implementation (compat mode)
    static const std::string compat_fm_version;

    /// Mark the end of an entry in binary output files (compat mode)
    static const int compat_new_entry;

    /// Use nonbonded exclusions (default: on)
    bool nonbonded_exclusion_on;

    /// Use bonded exclusions (default: on)
    bool bonded_exclusion_on;

    /// FDA groups index 1 (defined in pfi-file)
    int index_group1;

    /// FDA groups index 2 (defined in pfi-file)
    int index_group2;

    /// groups defined in pfn-file
    t_blocka* groups;

    /// groupnames defined in pfn-file
    char** groupnames;

    /// Forces lower than threshold will not be considered
    real threshold;

    /// Normalize punctual stress per residue (default: off)
    bool normalize_psr;

    /// If true unsupported potentials will be ignored.
    /// Be aware that the sum of distributed forces do not agree with the total forces.
    /// If false the rerun will be stopped if a unsupported potential is used.
    bool ignore_missing_potentials;

};

} // namespace fda

#endif /* SRC_GROMACS_FDA_FDASETTINGS_H_ */
