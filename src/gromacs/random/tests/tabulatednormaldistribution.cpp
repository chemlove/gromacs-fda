/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2015,2016, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \internal \file
 * \brief Tests for GROMACS tabulated normal distribution
 *
 * \author Erik Lindahl <erik.lindahl@gmail.com>
 * \ingroup module_random
 */
#include "gmxpre.h"

#include "gromacs/random/tabulatednormaldistribution.h"

#include <gtest/gtest.h>

#include "gromacs/random/threefry.h"

#include "testutils/refdata.h"
#include "testutils/testasserts.h"

namespace gmx
{

namespace
{

TEST(TabulatedNormalDistributionTest, Output14)
{
    gmx::test::TestReferenceData         data;
    gmx::test::TestReferenceChecker      checker(data.rootChecker());

    gmx::ThreeFry2x64<2>                 rng(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<>   dist(2.0, 5.0); // Use default 14-bit resolution
    std::vector<float>                   result;

    for (int i = 0; i < 10; i++)
    {
        result.push_back(dist(rng));
    }
    checker.checkSequence(result.begin(), result.end(), "TabulatedNormalDistribution14");
}

TEST(TabulatedNormalDistributionTest, Output16)
{
    gmx::test::TestReferenceData                  data;
    gmx::test::TestReferenceChecker               checker(data.rootChecker());

    gmx::ThreeFry2x64<2>                          rng(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<float, 16>   dist(2.0, 5.0); // Use larger 16-bit table
    std::vector<float>                            result;

    for (int i = 0; i < 10; i++)
    {
        result.push_back(dist(rng));
    }
    checker.checkSequence(result.begin(), result.end(), "TabulatedNormalDistribution16");
}

TEST(TabulatedNormalDistributionTest, OutputDouble14)
{
    gmx::test::TestReferenceData                  data;
    gmx::test::TestReferenceChecker               checker(data.rootChecker());

    gmx::ThreeFry2x64<2>                          rng(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<double>      dist(2.0, 5.0);
    std::vector<double>                           result;

    for (int i = 0; i < 10; i++)
    {
        result.push_back(dist(rng));
    }
    checker.setDefaultTolerance(test::ulpTolerance(15)); //compiler usage of FMA in makeTable can cause higher difference
    checker.checkSequence(result.begin(), result.end(), "TabulatedNormalDistributionDouble14");
}

TEST(TabulatedNormalDistributionTest, Logical)
{
    gmx::ThreeFry2x64<2>                 rng(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<>   distA(2.0, 5.0);
    gmx::TabulatedNormalDistribution<>   distB(2.0, 5.0);
    gmx::TabulatedNormalDistribution<>   distC(3.0, 5.0);
    gmx::TabulatedNormalDistribution<>   distD(2.0, 4.0);

    EXPECT_EQ(distA, distB);
    EXPECT_NE(distA, distC);
    EXPECT_NE(distA, distD);
}


TEST(TabulatedNormalDistributionTest, Reset)
{
    gmx::ThreeFry2x64<2>                                      rng(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<>                        distA(2.0, 5.0);
    gmx::TabulatedNormalDistribution<>                        distB(2.0, 5.0);
    gmx::TabulatedNormalDistribution<>::result_type           valA, valB;

    valA = distA(rng);

    distB(rng);
    rng.restart();
    distB.reset();

    valB = distB(rng);

    EXPECT_EQ(valA, valB);
}

TEST(TabulatedNormalDistributionTest, AltParam)
{
    gmx::ThreeFry2x64<2>                            rngA(123456, gmx::RandomDomain::Other);
    gmx::ThreeFry2x64<2>                            rngB(123456, gmx::RandomDomain::Other);
    gmx::TabulatedNormalDistribution<>              distA(2.0, 5.0);
    gmx::TabulatedNormalDistribution<>              distB;
    gmx::TabulatedNormalDistribution<>::param_type  paramA(2.0, 5.0);

    EXPECT_NE(distA(rngA), distB(rngB));
    rngA.restart();
    rngB.restart();
    distA.reset();
    distB.reset();
    EXPECT_EQ(distA(rngA), distB(rngB, paramA));
}

}      // namespace anonymous

}      // namespace gmx