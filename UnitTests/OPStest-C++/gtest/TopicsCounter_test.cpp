/**
*
* Copyright (C) 2026 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gtest/gtest.h"

#include "TopicsCounter.h"

TEST(Test_TopicsCounter, TestDefault) {

    ops::TopicsCounter cnt;

    EXPECT_TRUE(cnt.peek().empty());

    EXPECT_EQ(cnt.update("kalle", true), 1);
    EXPECT_EQ(cnt.peek().size(), 1u);
    EXPECT_EQ(cnt.update("kalle", true), 2);
    EXPECT_EQ(cnt.update("kalle", true), 3);
    EXPECT_EQ(cnt.peek().size(), 1u);
    EXPECT_EQ(cnt.update("olle", true), 1);
    EXPECT_EQ(cnt.peek().size(), 2u);

    EXPECT_EQ(cnt.update("kalle", false), 2);
    EXPECT_EQ(cnt.update("kalle", false), 1);
    EXPECT_EQ(cnt.update("kalle", false), 0);
    EXPECT_EQ(cnt.update("olle", false), 0);

    EXPECT_EQ(cnt.peek().size(), 2u);
}

TEST(Test_TopicsCounter, TestErase) {

    ops::TopicsCounter cnt(true);

    EXPECT_TRUE(cnt.peek().empty());

    EXPECT_EQ(cnt.update("kalle", true), 1);
    EXPECT_EQ(cnt.peek().size(), 1u);
    EXPECT_EQ(cnt.update("kalle", true), 2);
    EXPECT_EQ(cnt.update("kalle", true), 3);
    EXPECT_EQ(cnt.peek().size(), 1u);
    EXPECT_EQ(cnt.update("olle", true), 1);
    EXPECT_EQ(cnt.peek().size(), 2u);

    EXPECT_EQ(cnt.update("kalle", false), 2);
    EXPECT_EQ(cnt.update("kalle", false), 1);
    EXPECT_EQ(cnt.update("kalle", false), 0);
    EXPECT_EQ(cnt.peek().size(), 1u);
    EXPECT_EQ(cnt.update("olle", false), 0);

    EXPECT_EQ(cnt.peek().size(), 0u);
}
