#
# This file is part of the GROMACS molecular simulation package.
#
# Copyright (c) 2014,2015,2016,2017,2018, by the GROMACS development team, led by
# Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
# and including many others, as listed in the AUTHORS file in the
# top-level source directory and at http://www.gromacs.org.
#
# GROMACS is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1
# of the License, or (at your option) any later version.
#
# GROMACS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with GROMACS; if not, see
# http://www.gnu.org/licenses, or write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
#
# If you want to redistribute modifications to GROMACS, please
# consider that scientific software is very special. Version
# control is crucial - bugs must be traceable. We will be happy to
# consider code for inclusion in the official distribution, but
# derived work must not be called official GROMACS. Details are found
# in the README & COPYING files - if they are missing, get the
# official version at http://www.gromacs.org.
#
# To help us fund GROMACS development, we humbly ask that you cite
# the research papers on the package. Check out http://www.gromacs.org.

if(MANUAL_BUILD_IS_POSSIBLE)
    add_custom_command(
        OUTPUT ${SPHINX_LATEX_FILE}
        DEPENDS sphinx-programs sphinx-input
        COMMAND
            ${SPHINX_EXECUTABLE}
            -q -E -b latex
            -w sphinx-latex.log
            -d "${SPHINX_CACHE_DIR}"
            "${SPHINX_INPUT_DIR}"
            "${TEX_OUTPUT_DIR}"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Preparing PDF manual LaTeX input files"
        VERBATIM
        )


    LATEX_GET_OUTPUT_PATH(output_dir)

    # Avoid generating a global "html" target that clashes with the
    # html target from program generation of help text in share/html.
    set(LATEX2HTML_CONVERTER off)

    # This helps make the index look decent.
    set(MAKEINDEX_COMPILER_FLAGS -s ${CMAKE_CURRENT_SOURCE_DIR}/hfill.ist)
    # UseLATEX.cmake is hacked to call Anton Feenstra's subindex script
    # to improve the index generation and to use the Sphinx generated
    # gromacs.tex file. Don't replace UseLATEX.cmake
    # blindly from upstream!

    # An explicit "make manual" will still be required for the manual
    # to be built.

    # TODO The whole script needs to be part of a subprocess called at make
    # time so that the generated gromacs.tex file is available for ADD_LATEX_DOCUMENT
    # when it searches for it.
    # Note: The gromacs.tex file is not actually copied by the UseLatex.cmake script,
    # but generated by Sphinx during the make step. 
    ADD_LATEX_DOCUMENT(gromacs.tex
        DEPENDS ${SPHINX_LATEX_FILE}
        DEFAULT_PDF
        )

    # Finally, the command to build the manual.

    add_custom_target(manual
        DEPENDS pdf
        )
else()
        add_custom_target(manual
        COMMAND ${CMAKE_COMMAND} -E echo
        "Cannot build PDF manual, because ${MANUAL_BUILD_NOT_POSSIBLE_REASON}"
        VERBATIM)
endif()
