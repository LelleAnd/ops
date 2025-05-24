/**
*
* Copyright (C) 2018-2025 Lennart Andersson.
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

#include <cstdint>

#include "gtest/gtest.h"

#undef FIXED_NO_STD_STRING

#include "fixed_string_support.h"

// ===============================
// Helper class to test template "construct from any T with c_str() & size()"

class mstring
{
public:
	size_t size() const { return strlen(internal); }
	const char* c_str() const { return &internal[0]; }

	mstring() { internal[0] = '\0'; }
	mstring(const char* const val) 
	{ 
		int const len = ((int)strlen(val) < 79) ? (int)strlen(val) : 79; 
		for (auto i = 0; i < len; i++) { internal[i] = val[i]; internal[len] = '\0'; }
	}

	~mstring() = default;
	mstring(const mstring& other) = default;
	mstring& operator= (const mstring& other) = default;
	mstring(mstring&& other) = delete;
	mstring& operator =(mstring&& other) = delete;
private:
	char internal[80];
};

// ===============================

using namespace ops::strings;

TEST(Test_fixed_string, TestConstructors) {

	fixed_string<50> a;		// default constructor

	EXPECT_EQ(a.size(), (size_t)0) << "Wrong size";
	EXPECT_EQ(a.length(), (size_t)0) << "Wrong length";
	EXPECT_EQ(a.max_size(), (size_t)50) << "Wrong max_size";
	EXPECT_TRUE(a.empty()) << "Not empty";
	EXPECT_STREQ(a.c_str(), "") << "Content error";
	EXPECT_STREQ(a.data(), "") << "Content error";

	fixed_string<20> b("hej hopp");		// const char*

	EXPECT_EQ(b.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(b.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(b.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(b.empty()) << "Empty";
	EXPECT_STREQ(b.c_str(), "hej hopp") << "Content error";
	EXPECT_STREQ(b.data(), "hej hopp") << "Content error";

	char b1_source[] = "hej hopp";
	fixed_string<20> b1(b1_source);		// char*

	EXPECT_EQ(b1.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(b1.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(b1.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(b1.empty()) << "Empty";
	EXPECT_STREQ(b1.c_str(), "hej hopp") << "Content error";
	EXPECT_STREQ(b1.data(), "hej hopp") << "Content error";

	fixed_string<20> bcc("hej", " hopp"); // const char*, const char*

	EXPECT_EQ(bcc.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(bcc.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(bcc.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(bcc.empty()) << "Empty";
	EXPECT_STREQ(bcc.c_str(), "hej hopp") << "Content error";
	EXPECT_STREQ(bcc.data(), "hej hopp") << "Content error";

	char bcc1_source[] = "Hello ";
	char bcc2_source[] = "World";
	fixed_string<20> bcc1(bcc1_source, bcc2_source); // char*, char*

	EXPECT_EQ(bcc1.size(), (size_t)11) << "Wrong size";
	EXPECT_EQ(bcc1.length(), (size_t)11) << "Wrong length";
	EXPECT_EQ(bcc1.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(bcc1.empty()) << "Empty";
	EXPECT_STREQ(bcc1.c_str(), "Hello World") << "Content error";
	EXPECT_STREQ(bcc1.data(), "Hello World") << "Content error";

	fixed_string<20> b2("Det var en gång", 3); // const char*, length

	EXPECT_EQ(b2.size(), (size_t)3) << "Wrong size";
	EXPECT_EQ(b2.length(), (size_t)3) << "Wrong length";
	EXPECT_EQ(b2.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(b2.empty()) << "Empty";
	EXPECT_STREQ(b2.c_str(), "Det") << "Content error";

	char b2a_source[] = "Detta";
	fixed_string<20> b2a(b2a_source, 3);		// char*, length

	EXPECT_EQ(b2a.size(), (size_t)3) << "Wrong size";
	EXPECT_EQ(b2a.length(), (size_t)3) << "Wrong length";
	EXPECT_EQ(b2a.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(b2a.empty()) << "Empty";
	EXPECT_STREQ(b2a.c_str(), "Det") << "Content error";

#ifndef FIXED_NO_STD_STRING
	std::string kalle("kalle");
	fixed_string<15> e(kalle);		// std::string (if enabled)

	EXPECT_EQ(e.size(), (size_t)5) << "Wrong size";
	EXPECT_EQ(e.length(), (size_t)5) << "Wrong length";
	EXPECT_EQ(e.max_size(), (size_t)15) << "Wrong max_size";
	EXPECT_FALSE(e.empty()) << "Empty";
	EXPECT_STREQ(e.c_str(), kalle.c_str()) << "Content error";

#ifdef FIXED_C17_DETECTED
	//                         0----56789----
	std::string anotherstring("Another string");
	std::string_view sv(&anotherstring[5], 5);
	fixed_string<10> svf(sv);
	EXPECT_EQ(svf.size(), (size_t)5) << "Wrong size";
	EXPECT_EQ(svf.length(), (size_t)5) << "Wrong length";
	EXPECT_EQ(svf.max_size(), (size_t)10) << "Wrong max_size";
	EXPECT_FALSE(svf.empty()) << "Empty";
	EXPECT_STREQ(svf.c_str(), "er st") << "Content error";
#endif
#endif

	fixed_string<80> c2(b);		// template constructor fixed_string<M>

	EXPECT_EQ(c2.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(c2.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(c2.max_size(), (size_t)80) << "Wrong max_size";
	EXPECT_FALSE(c2.empty()) << "Empty";
	EXPECT_STREQ(c2.c_str(), "hej hopp") << "Content error";

	// Test template constructor "from any T..."
	mstring g1("hej");
	fixed_string<100> f1(g1);

	EXPECT_EQ(f1.size(), (size_t)3) << "Wrong size";
	EXPECT_EQ(f1.length(), (size_t)3) << "Wrong length";
	EXPECT_EQ(f1.max_size(), (size_t)100) << "Wrong max_size";
	EXPECT_FALSE(f1.empty()) << "Empty";
	EXPECT_STREQ(f1.c_str(), "hej") << "Content error";

	fixed_string<20> c(b);		// copy constructor

	EXPECT_EQ(c.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(c.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(c.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(c.empty()) << "Empty";
	EXPECT_STREQ(c.c_str(), "hej hopp") << "Content error";

	fixed_string<20> d = c;		// assignment operator

	EXPECT_EQ(d.size(), (size_t)8) << "Wrong size";
	EXPECT_EQ(d.length(), (size_t)8) << "Wrong length";
	EXPECT_EQ(d.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(d.empty()) << "Empty";
	EXPECT_STREQ(d.c_str(), "hej hopp") << "Content error";

#ifdef FIXED_C17_DETECTED
	// Compile time constexpr strings
	constexpr static ops::strings::fixed_string<20, ops::strings::overrun_policy_t::truncate_string> fs{ "Hello" };
	static_assert(fs.max_size() == 20);
	static_assert(fs.size() == 5);
	static_assert(std::string_view(fs.c_str()) == "Hello");

	constexpr static ops::strings::fixed_string_trunc<30> fs2{ fs.data() };
	static_assert(fs2.max_size() == 30);
	static_assert(fs2.size() == 5);
	static_assert(std::string_view(fs2.c_str()) == "Hello");

	constexpr auto fs3 = fs + fs2;
	static_assert(fs3.max_size() == 50);
	static_assert(fs3.size() == 10);
	static_assert(std::string_view(fs3.c_str()) == "HelloHello");

	constexpr static ops::strings::fixed_string_trunc<40> fs4{ fs.data(), " World" };
	static_assert(fs4.max_size() == 40);
	static_assert(fs4.size() == 11);
	static_assert(std::string_view(fs4.c_str()) == "Hello World");

	constexpr std::string_view aaa(fs4.c_str(), 5);

	constexpr static ops::strings::fixed_string_trunc<50> fs5(aaa);
	static_assert(fs5.max_size() == 50);
	static_assert(fs5.size() == 5);
	static_assert(std::string_view(fs5.c_str()) == "Hello");

	constexpr static std::string_view bbb(fs4);

	constexpr auto fs6 = ops::strings::make_fixed_string("Hej Hopp");
	static_assert(fs6.max_size() == 8);
	static_assert(fs6.size() == 8);
	static_assert(std::string_view(fs6.c_str()) == "Hej Hopp");
	static_assert(sizeof(fs6) == 10);
#endif
}

TEST(Test_fixed_string, TestModifiers) {
	fixed_string<50> a;
	fixed_string<20> const b("hej hopp");
	fixed_string<20> b2("Det var en gång", 3);
	std::string kalle("kalle");
	fixed_string<15> e(kalle);

	// Test modifiers
	e += ' ';
	e += b;

	EXPECT_EQ(e.size(), (size_t)14) << "Wrong size";
	EXPECT_EQ(e.length(), (size_t)14) << "Wrong length";
	EXPECT_EQ(e.max_size(), (size_t)15) << "Wrong max_size";
	EXPECT_FALSE(e.empty()) << "Empty";
	EXPECT_STREQ(e.c_str(), "kalle hej hopp") << "Content error";

	b2 += ' ';
	b2 += kalle;

	EXPECT_EQ(b2.size(), (size_t)9) << "Wrong size";
	EXPECT_EQ(b2.length(), (size_t)9) << "Wrong length";
	EXPECT_EQ(b2.max_size(), (size_t)20) << "Wrong max_size";
	EXPECT_FALSE(b2.empty()) << "Empty";
	EXPECT_STREQ(b2.c_str(), "Det kalle") << "Content error";

	a += e;
	a += " i lingonskogen";

	EXPECT_EQ(a.size(), (size_t)29) << "Wrong size";
	EXPECT_EQ(a.length(), (size_t)29) << "Wrong length";
	EXPECT_EQ(a.max_size(), (size_t)50) << "Wrong max_size";
	EXPECT_FALSE(a.empty()) << "Empty";
	//                               1    1    2    2  2
	//                     0    5    0    5    0    5  8
	EXPECT_STREQ(a.c_str(), "kalle hej hopp i lingonskogen") << "Content error";
}

TEST(Test_fixed_string, TestSubstr) {
	//                            1    1    2    2  2
	//                  0    5    0    5    0    5  8
	fixed_string<50> a("kalle hej hopp i lingonskogen");

	// Test substr()
	fixed_string<10> f = a.substr(24);

	EXPECT_EQ(f.size(), (size_t)5) << "Wrong size";
	EXPECT_EQ(f.length(), (size_t)5) << "Wrong length";
	EXPECT_EQ(f.max_size(), (size_t)10) << "Wrong max_size";
	EXPECT_STREQ(f.c_str(), "kogen") << "Content error";

	f.clear();

	EXPECT_EQ(f.size(), (size_t)0) << "Wrong size";
	EXPECT_EQ(f.length(), (size_t)0) << "Wrong length";
	EXPECT_EQ(f.max_size(), (size_t)10) << "Wrong max_size";
	EXPECT_TRUE(f.empty()) << "Not empty";
	EXPECT_STREQ(f.c_str(), "") << "Content error";

	auto g = a.substr<5>(10, 4);

	EXPECT_EQ(g.size(), (size_t)4) << "Wrong size";
	EXPECT_EQ(g.length(), (size_t)4) << "Wrong length";
	EXPECT_EQ(g.max_size(), (size_t)5) << "Wrong max_size";
	EXPECT_STREQ(g.c_str(), "hopp") << "Content error";

	EXPECT_EQ(a[0], 'k') << "Wrong character";
	EXPECT_EQ(a.at(10), 'h') << "Wrong character";

	std::string zz = a.substr();
	EXPECT_STREQ(zz.c_str(), "kalle hej hopp i lingonskogen") << "Content error";
	EXPECT_STREQ(a.c_str(), "kalle hej hopp i lingonskogen") << "Content error";
}

TEST(Test_fixed_string, TestResize) {
	// Test resize()
	//                             1    1    2    2  2
	//                   0    5    0    5    0    5  8
	fixed_string<100> h("kalle hej hopp i lingonskogen");
	EXPECT_EQ(h.size(), (size_t)29) << "Wrong size";
	EXPECT_EQ(h.length(), (size_t)29) << "Wrong length";
	EXPECT_EQ(h.max_size(), (size_t)100) << "Wrong max_size";

	h.resize(34, 'Z');

	EXPECT_EQ(h.size(), (size_t)34) << "Wrong size";
	EXPECT_EQ(h.length(), (size_t)34) << "Wrong length";
	EXPECT_EQ(h.max_size(), (size_t)100) << "Wrong max_size";
	EXPECT_STREQ(h.c_str(), "kalle hej hopp i lingonskogenZZZZZ") << "Content error";

	h.resize(32);

	EXPECT_EQ(h.size(), (size_t)32) << "Wrong size";
	EXPECT_EQ(h.length(), (size_t)32) << "Wrong length";
	EXPECT_EQ(h.max_size(), (size_t)100) << "Wrong max_size";
	EXPECT_STREQ(h.c_str(), "kalle hej hopp i lingonskogenZZZ") << "Content error";

	h[14] = '\0';
	h.resize();

	EXPECT_EQ(h.size(), (size_t)14) << "Wrong size";
	EXPECT_EQ(h.length(), (size_t)14) << "Wrong length";
	EXPECT_EQ(h.max_size(), (size_t)100) << "Wrong max_size";
	EXPECT_STREQ(h.c_str(), "kalle hej hopp") << "Content error";
}

TEST(Test_fixed_string, TestFind) {
	// Test find()
	//                             1    1    2    2 2
	//                   0    5    0    5    0    5 7
	fixed_string<100> h("kalle hej hoppkalle hej hopp");
	std::string kalle("kalle");

	fixed_string<100>::size_type npos = fixed_string<100>::npos;

	EXPECT_EQ(h.find('h'), (size_t)6) << "find failed";
	EXPECT_EQ(h.find('h', 7), (size_t)10) << "find failed";
	EXPECT_EQ(h.find('x'), npos) << "find failed";
	EXPECT_EQ(h.find('k', 90), npos) << "find failed";
	EXPECT_EQ(h.find("hej"), (size_t)6) << "find failed";
	EXPECT_EQ(h.find("hop"), (size_t)10) << "find failed";
	EXPECT_EQ(h.find("zzz"), npos) << "find failed";
	EXPECT_EQ(h.find("hop", 28), npos) << "find failed";
	EXPECT_EQ(h.find("hop", 11), (size_t)24) << "find failed";
	EXPECT_EQ(h.find(kalle), (size_t)0) << "find failed";
	EXPECT_EQ(h.find(kalle, 1), (size_t)14) << "find failed";

	EXPECT_EQ(h.find_first_of('j'), (size_t)8) << "find failed";
	EXPECT_EQ(h.find_first_of('j', 9), (size_t)22) << "find failed";
	EXPECT_EQ(h.find_first_of('j', 100), npos) << "find failed";
	EXPECT_EQ(h.find_first_of('z'), npos) << "find failed";
	EXPECT_EQ(h.find_first_not_of('k'), (size_t)1) << "find failed";
	EXPECT_EQ(h.find_first_not_of('p', 12), (size_t)14) << "find failed";
	EXPECT_EQ(h.find_first_not_of('j', 100), npos) << "find failed";

	EXPECT_EQ(h.find_first_of("j :"), (size_t)5) << "find failed";
	EXPECT_EQ(h.find_first_of("poh", 4), (size_t)6) << "find failed";
	EXPECT_EQ(h.find_first_of("xzy", 20), npos) << "find failed";

	EXPECT_EQ(h.find_last_of('j'), (size_t)22) << "find failed";
	EXPECT_EQ(h.find_last_of('j', 21), (size_t)8) << "find failed";
	EXPECT_EQ(h.find_last_of('j', 200), (size_t)22) << "find failed";
	EXPECT_EQ(h.find_last_of('z'), npos) << "find failed";
	EXPECT_EQ(h.find_last_not_of('p'), (size_t)25) << "find failed";
	EXPECT_EQ(h.find_last_not_of('p', 13), (size_t)11) << "find failed";
	EXPECT_EQ(h.find_last_not_of('p', 150), (size_t)25) << "find failed";

	fixed_string<90> x("==================");
	EXPECT_EQ(x.find_last_of('s'), npos) << "find failed";
	EXPECT_EQ(x.find_last_not_of('='), npos) << "find failed";

	fixed_string<90> xx("s=================");
	EXPECT_EQ(xx.find_last_of('s'), (size_t)0) << "find failed";
	EXPECT_EQ(xx.find_last_not_of('='), (size_t)0) << "find failed";

	fixed_string<90> y("");
	EXPECT_EQ(y.find_last_of('s'), npos) << "find failed";
	EXPECT_EQ(y.find_last_not_of('='), npos) << "find failed";
}

TEST(Test_fixed_string, TestExceptions) {
	// Exception handling
	EXPECT_NO_THROW( fixed_string<3> a("hhh") );
	EXPECT_THROW( fixed_string<3> a("hhhh"), fixed_string<3>::size_out_of_range );

	EXPECT_NO_THROW( fixed_string<3> a("hhhh", 3) );
	EXPECT_THROW( fixed_string<3> a("hhhh", 4), fixed_string<3>::size_out_of_range );

	EXPECT_NO_THROW( fixed_string<3> a(std::string("eee")) );
	EXPECT_THROW( fixed_string<3> a(std::string("eeee")), fixed_string<3>::size_out_of_range );

	EXPECT_NO_THROW( {fixed_string<3> a(std::string("eee")); a.resize(3); } );
	EXPECT_THROW( { fixed_string<3> a(std::string("eee")); a.resize(4); }, fixed_string<3>::size_out_of_range );

	EXPECT_NO_THROW({ fixed_string<3> a("hhh"); char b = a[3]; (void)(b); });
	EXPECT_THROW( { fixed_string<3> a("hhh"); char b = a[4]; (void)(b); }, fixed_string<3>::index_out_of_range);

	EXPECT_NO_THROW( { fixed_string<3> a("hhh"); char b = a.at(2); (void)(b); } );
	EXPECT_THROW( { fixed_string<3> a("hhh"); char b = a.at(3); (void)(b); }, fixed_string<3>::index_out_of_range);

	EXPECT_NO_THROW( { fixed_string<3> a("hh"); a += 'h'; } );
	EXPECT_THROW( { fixed_string<3> a("hhh"); a += 'h'; }, fixed_string<3>::size_out_of_range);

	EXPECT_NO_THROW( { fixed_string<3> a("h"); a += "hh"; } );
	EXPECT_THROW( { fixed_string<3> a("hh"); a += "hh"; }, fixed_string<3>::size_out_of_range);
}

TEST(Test_fixed_string, TestImplicitConversion) {

	fixed_string<100> a("kalle hej hopp i lingonskogen");
	std::string st = a;
	EXPECT_STREQ(st.c_str(), "kalle hej hopp i lingonskogen") << "Content error";

#ifdef FIXED_C17_DETECTED
	std::string_view sv = a;
	EXPECT_STREQ(sv.data(), "kalle hej hopp i lingonskogen") << "Content error";
#endif
}

TEST(Test_fixed_string, TestRelationalOperators) {
	// Test relational operators
	std::string kalle("kalle");
	fixed_string<20> m(kalle);
	fixed_string<30> n(kalle);
	fixed_string<30> o(kalle);
	fixed_string<40> p("hej hopp");

	EXPECT_TRUE(m == n) << "Not equal";
	EXPECT_TRUE(n == o) << "Not equal";
	EXPECT_FALSE(m == p) << "Equal";

	EXPECT_FALSE(m != n) << "Not Equal";
	EXPECT_FALSE(n != o) << "Not Equal";
	EXPECT_TRUE(m != p) << "Equal";

	EXPECT_TRUE(p < m) << "Not less than";
	EXPECT_FALSE(m < n) << "Not less than";

	EXPECT_TRUE(p <= m) << "Not less than";
	EXPECT_TRUE(m <= n) << "Not less than";
	EXPECT_FALSE(m <= p) << "Not less than";

	EXPECT_TRUE(m > p) << "Not larger than";
	EXPECT_FALSE(m > o) << "Not larger than";

	EXPECT_TRUE(m >= p) << "Not larger than";
	EXPECT_TRUE(m >= n) << "Not larger than";
	EXPECT_FALSE(p >= m) << "Not larger than";
}

TEST(Test_fixed_string, TestPlusOperator) {
	// Test operator +
	std::string kalle("kalle");
	fixed_string<30> o(kalle);
	fixed_string<40> p("hej hopp");

	auto r = p + o;
	EXPECT_EQ(r.size(), (size_t)13) << "Wrong size";
	EXPECT_EQ(r.length(), (size_t)13) << "Wrong length";
	EXPECT_EQ(r.max_size(), (size_t)70) << "Wrong max_size";
	EXPECT_STREQ(r.c_str(), "hej hoppkalle") << "Content error";
}

TEST(Test_fixed_string_support, TestStreamOperator) {

	fixed_string<100> const a("kalle hej hopp i lingonskogen");
	std::stringstream ss;
	ss << a;
	EXPECT_STREQ(ss.str().c_str(), "kalle hej hopp i lingonskogen") << "Content error";
}

TEST(Test_fixed_string_support, TestUpperLowerTrim) {

	fixed_string<40> const Aa("  Hej Hopp i Lingonskogen  ");
	fixed_string<40> aa, AA, Tt;
	aa = ToLower(Aa);
	EXPECT_STREQ(aa.c_str(), "  hej hopp i lingonskogen  ") << "ToLower Failed";

	AA = ToUpper(Aa);
	EXPECT_STREQ(AA.c_str(), "  HEJ HOPP I LINGONSKOGEN  ") << "ToUpper failed";

	Tt = Trim(Aa);
	EXPECT_STREQ(Tt.c_str(), "Hej Hopp i Lingonskogen") << "Trim failed";
}

TEST(Test_fixed_string_trunc, Test) {
	// Test constructors
	fixed_string_trunc<4> a;

	EXPECT_EQ(a.size(), (size_t)0) << "Wrong size";
	EXPECT_EQ(a.length(), (size_t)0) << "Wrong length";
	EXPECT_EQ(a.max_size(), (size_t)4) << "Wrong max_size";
	EXPECT_TRUE(a.empty()) << "Not empty";
	EXPECT_STREQ(a.c_str(), "") << "Content error";
	EXPECT_STREQ(a.data(), "") << "Content error";

	a += "123456789";
	EXPECT_EQ(a.size(), (size_t)4) << "Wrong size";
	EXPECT_STREQ(a.c_str(), "1234") << "Content error";

	fixed_string_trunc<5> b("hej hopp");

	EXPECT_EQ(b.size(), (size_t)5) << "Wrong size";
	EXPECT_EQ(b.max_size(), (size_t)5) << "Wrong max_size";
	EXPECT_FALSE(b.empty()) << "Empty";
	EXPECT_STREQ(b.c_str(), "hej h") << "Content error";

	b += "jgjhgj";
	EXPECT_EQ(b.size(), (size_t)5) << "Wrong size";
	EXPECT_STREQ(b.c_str(), "hej h") << "Content error";

	fixed_string_trunc<7> b2("Det", 7);

	EXPECT_EQ(b2.size(), (size_t)3) << "Wrong size";
	EXPECT_EQ(b2.max_size(), (size_t)7) << "Wrong max_size";
	EXPECT_FALSE(b2.empty()) << "Empty";
	EXPECT_STREQ(b2.c_str(), "Det") << "Content error";

	b2 += " var en gång";
	EXPECT_EQ(b2.size(), (size_t)7) << "Wrong size";
	EXPECT_STREQ(b2.c_str(), "Det var") << "Content error";

	fixed_string_trunc<10> c(b);

	EXPECT_EQ(c.size(), (size_t)5) << "Wrong size";
	EXPECT_EQ(c.max_size(), (size_t)10) << "Wrong max_size";
	EXPECT_FALSE(c.empty()) << "Empty";
	EXPECT_STREQ(c.c_str(), "hej h") << "Content error";

	c += "opp i lingonskogen";
	EXPECT_EQ(c.size(), (size_t)10) << "Wrong size";
	EXPECT_STREQ(c.c_str(), "hej hopp i") << "Content error";

	fixed_string_trunc<6> d = c;

	EXPECT_EQ(d.size(), (size_t)6) << "Wrong size";
	EXPECT_EQ(d.length(), (size_t)6) << "Wrong length";
	EXPECT_EQ(d.max_size(), (size_t)6) << "Wrong max_size";
	EXPECT_FALSE(d.empty()) << "Empty";
	EXPECT_STREQ(d.c_str(), "hej ho") << "Content error";
}

#ifdef not_used_test_code
// -------------------------------------

void Test_s(std::string value)
{
}

void Test_f(fixed_string<40> value)
{
}

void Test_ft(fixed_string_trunc<40> value)
{
}

fixed_string<50> TestFunc()
{
	return "hopp";
}

void Test()
{
	fixed_string<42> kalle("hej hopp");
	Test_s(kalle);
	std::string localStr = TestFunc();
	Test_f(localStr);
}

// -----------------------------------------

void Test2()
{
	mstring a("hej");
	std::string b("kalle");
	fixed_string<30> c("skog");
	Test_f(a);
	Test_f(b);
	Test_f(c);
	Test_ft(a);
	Test_ft(b);
	Test_ft(c);
}
#endif
