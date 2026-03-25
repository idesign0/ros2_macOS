/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>
#include "gz/common/Console.hh"
#include "gz/common/URI.hh"

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
TEST(URITEST, URIPath)
{
  URIPath path1, path2, path3;
  EXPECT_TRUE(path1.Str().empty());
  EXPECT_FALSE(path1.IsAbsolute());

  path1.PushFront("part1");
  EXPECT_EQ(path1.Str(), "part1");
  path1.PushBack("part2");
  EXPECT_EQ(path1.Str(), "part1/part2");
  EXPECT_FALSE(path1.IsAbsolute());

  path1.PushFront("part 0");
  EXPECT_EQ(path1.Str(), "part 0/part1/part2");
  // TODO(anyone): switch to the following once URI class is upgraded
  // path1.PushFront("part%200");
  // EXPECT_EQ(path1.Str(), "part%200/part1/part2");
  EXPECT_FALSE(path1.IsAbsolute());

  path2 = path1 / "part3";
  EXPECT_EQ(path2.Str(), "part 0/part1/part2/part3");
  // TODO(anyone): switch to the following once URI class is upgraded
  // EXPECT_EQ(path2.Str(), "part%200/part1/part2/part3");
  EXPECT_FALSE(path2.IsAbsolute());

  path1 /= "part3";
  EXPECT_EQ(path1.Str(), "part 0/part1/part2/part3");
  // TODO(anyone): switch to the following once URI class is upgraded
  // EXPECT_EQ(path1.Str(), "part%200/part1/part2/part3");
  EXPECT_FALSE(path1.IsAbsolute());

  EXPECT_TRUE(path1 == path2);

  path3 = path1;
  EXPECT_TRUE(path3 == path1);
  EXPECT_FALSE(path3.IsAbsolute());

  path1.Clear();
  EXPECT_TRUE(path1.Str().empty());

  URIPath path4(path3);
  EXPECT_TRUE(path4 == path3);

  URIPath path5("/absolute/path");
  EXPECT_EQ(path5.Str(), "/absolute/path");
  EXPECT_TRUE(path5.IsAbsolute());

  URIPath path6("/");
  EXPECT_EQ(path6.Str(), "/");
  EXPECT_TRUE(path6.IsAbsolute());

  EXPECT_TRUE(path6.PopFront().empty());
  EXPECT_TRUE(path6.PopBack().empty());

  URIPath path7;
  path7.PushFront("/abs");
  EXPECT_EQ(path7.Str(), "/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.PushFront("/abs2");
  EXPECT_EQ(path7.Str(), "/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.PushFront("abs3");
  EXPECT_EQ(path7.Str(), "/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.PushFront("");
  EXPECT_EQ(path7.Str(), "/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.PushFront("/");
  EXPECT_EQ(path7.Str(), "/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.SetRelative();
  EXPECT_EQ(path7.Str(), "abs3/abs2/abs");
  EXPECT_FALSE(path7.IsAbsolute());

  path7.PushFront("/");
  EXPECT_EQ(path7.Str(), "/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.SetRelative();
  path7.SetAbsolute();
  EXPECT_EQ(path7.Str(), "/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.SetAbsolute(false);
  path7.PushFront("/abs4");
  EXPECT_EQ(path7.Str(), "/abs4/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  path7.PushFront("abs6/abs5");
  EXPECT_EQ(path7.Str(), "/abs6%2Fabs5/abs4/abs3/abs2/abs");
  EXPECT_TRUE(path7.IsAbsolute());

  EXPECT_EQ("abs6%2Fabs5", path7.PopFront());
  EXPECT_EQ("/abs4/abs3/abs2/abs", path7.Str());
  EXPECT_EQ("abs", path7.PopBack());
  EXPECT_EQ("/abs4/abs3/abs2", path7.Str());

  URIPath path8;
  path8.PushBack("/abs");
  EXPECT_EQ(path8.Str(), "/abs");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.PushBack("/abs2");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.PushBack("abs3");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2/abs3");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.PushBack("");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2/abs3");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.PushBack("/");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2/abs3/%2F");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.SetRelative();
  EXPECT_EQ(path8.Str(), "abs/%2Fabs2/abs3/%2F");
  EXPECT_FALSE(path8.IsAbsolute());

  path8.PushBack("/");
  EXPECT_EQ(path8.Str(), "abs/%2Fabs2/abs3/%2F/%2F");
  EXPECT_FALSE(path8.IsAbsolute());

  path8.SetAbsolute();
  path8.PushBack("abs4");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2/abs3/%2F/%2F/abs4");
  EXPECT_TRUE(path8.IsAbsolute());

  path8.PushBack("abs5/abs6");
  EXPECT_EQ(path8.Str(), "/abs/%2Fabs2/abs3/%2F/%2F/abs4/abs5%2Fabs6");
  EXPECT_TRUE(path8.IsAbsolute());

  URIPath path9 = path8;
  EXPECT_EQ(path9.Str(), path8.Str());
  EXPECT_TRUE(path9.IsAbsolute());

  URIPath path10;
  path10.PushFront("C:");
  EXPECT_EQ(path10.Str(), "C:");
  EXPECT_TRUE(path10.IsAbsolute());

  path10.PushBack("Users");
  EXPECT_EQ(path10.Str(), "C:/Users");
  EXPECT_TRUE(path10.IsAbsolute());

  // Setting a C: path to relative is illegal
  path10.SetRelative();
  EXPECT_TRUE(path10.IsAbsolute());
}

/////////////////////////////////////////////////
TEST(URITEST, URIPathCoverageExtra)
{
  // getting full destructor coverage
  URIPath *p = new URIPath;
  ASSERT_NE(nullptr, p);
  delete p;
}

/////////////////////////////////////////////////
TEST(URITEST, URIPathString)
{
  URIPath path;
  EXPECT_TRUE(URIPath::Valid(""));
  EXPECT_TRUE(URIPath::Valid("//"));
  EXPECT_TRUE(URIPath::Valid("a//b"));
  EXPECT_TRUE(URIPath::Valid("/a//b"));
  EXPECT_FALSE(URIPath::Valid(" "));
  EXPECT_FALSE(URIPath::Valid("?invalid"));
  EXPECT_FALSE(URIPath::Valid("=invalid"));
  EXPECT_FALSE(URIPath::Valid("&invalid"));
  EXPECT_FALSE(URIPath::Valid("invalid#"));
  EXPECT_FALSE(URIPath::Valid("#invalid"));
  EXPECT_TRUE(URIPath::Valid("part1"));
  EXPECT_TRUE(URIPath::Valid("/"));
  EXPECT_TRUE(URIPath::Valid("/part1"));
  EXPECT_TRUE(URIPath::Valid("/part1/"));
  EXPECT_TRUE(URIPath::Valid("/part1/part2"));
  EXPECT_TRUE(URIPath::Valid("/part1/part2/"));
  EXPECT_TRUE(URIPath::Valid("C:"));
  EXPECT_TRUE(URIPath::Valid("C:/Users"));

  EXPECT_TRUE(URIPath::Valid("/part 1/part 2/"));
  // TODO(anyone): switch to the following once URI class is upgraded
  // EXPECT_FALSE(URIPath::Valid("/part 1/part 2/"));
  // EXPECT_TRUE(URIPath::Valid("/part+1/part+2"));
  // EXPECT_TRUE(URIPath::Valid("/part%201/part%202"));

  EXPECT_TRUE(path.Parse(""));
  EXPECT_TRUE(path.Parse("//"));
  EXPECT_TRUE(path.Parse("a//b"));
  EXPECT_TRUE(path.Parse("/a//b"));
  EXPECT_FALSE(path.Parse(" "));
  EXPECT_FALSE(path.Parse("?invalid"));
  EXPECT_FALSE(path.Parse("=invalid"));
  EXPECT_FALSE(path.Parse("&invalid"));
  EXPECT_FALSE(path.Parse("invalid#"));
  EXPECT_FALSE(path.Parse("#invalid"));
  EXPECT_TRUE(path.Parse("part1"));
  EXPECT_TRUE(path.Parse("/"));
  EXPECT_TRUE(path.Parse("/part1"));
  EXPECT_TRUE(path.Parse("/part1/"));
  EXPECT_TRUE(path.Parse("/part1/part2"));
  EXPECT_TRUE(path.Parse("/part1/part2/"));
  EXPECT_TRUE(path.Parse("C:"));
  EXPECT_TRUE(path.Parse("C:/Users"));

  EXPECT_TRUE(path.Parse("/part 1/part 2/"));
  // TODO(anyone): switch to the following once URI class is upgraded
  // EXPECT_FALSE(path.Parse("/part 1/part 2/"));
  // EXPECT_TRUE(path.Parse("/part+1/part+2"));
  // EXPECT_TRUE(path.Parse("/part%201/part%202"));

  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("//").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("a//b").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("/a//b").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath(" ").Valid()));

  // The following are valid because setting a URIPath to an invalid string
  // results in the path being empty, which in turn is a valid path.
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("?invalid").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("=invalid").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("&invalid").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("invalid#").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URIPath("#invalid").Valid()));

  EXPECT_NO_THROW(URIPath("/"));
  EXPECT_NO_THROW(URIPath("part1"));
  EXPECT_NO_THROW(URIPath("/part1"));
  EXPECT_NO_THROW(URIPath("/part1/"));
  EXPECT_NO_THROW(URIPath("/part1/part2"));
  EXPECT_NO_THROW(URIPath("/part 1/part2/"));
  EXPECT_NO_THROW(URIPath("C:"));
  EXPECT_NO_THROW(URIPath("C:/Users"));

  // TODO(anyone): uncomment following once URI class is upgraded
  // EXPECT_NO_THROW(URIPath("/part+1/part+2"));
  // EXPECT_NO_THROW(URIPath("/part%201/part%202"));
}

/////////////////////////////////////////////////
TEST(URITEST, URIQuery)
{
  URIQuery query1, query2, query3;
  EXPECT_TRUE(query1.Str().empty());

  query1.Insert("key1", "value1");
  EXPECT_EQ(query1.Str(), "?key1=value1");
  query1.Insert("key2", "value2");
  EXPECT_EQ(query1.Str(), "?key1=value1&key2=value2");

  query2 = query1;
  EXPECT_EQ(query2.Str(), query1.Str());
  EXPECT_TRUE(query2 == query1);

  query3 = query1;
  EXPECT_TRUE(query3 == query1);

  query1.Clear();
  EXPECT_TRUE(query1.Str().empty());

  URIQuery query4(query2);
  EXPECT_TRUE(query4 == query2);
}

/////////////////////////////////////////////////
TEST(URITEST, URIQueryCoverageExtra)
{
  // getting full destructor coverage
  URIQuery *p = new URIQuery;
  ASSERT_NE(nullptr, p);
  delete p;
}

/////////////////////////////////////////////////
TEST(URITEST, URIQueryString)
{
  URIQuery query;
  EXPECT_TRUE(query.Valid());

  // test static Valid function
  EXPECT_TRUE(URIQuery::Valid("??"));
  EXPECT_FALSE(URIQuery::Valid("invalid?"));
  EXPECT_TRUE(URIQuery::Valid("?valid?"));
  EXPECT_FALSE(URIQuery::Valid("? invalid"));
  EXPECT_FALSE(URIQuery::Valid("#invalid"));
  EXPECT_FALSE(URIQuery::Valid("invalid#"));
  EXPECT_TRUE(URIQuery::Valid("?key"));
  EXPECT_TRUE(URIQuery::Valid("?key="));
  EXPECT_TRUE(URIQuery::Valid("?=value"));
  EXPECT_FALSE(URIQuery::Valid("?key=value with space"));

  EXPECT_TRUE(URIQuery::Valid(""));
  EXPECT_TRUE(URIQuery::Valid("?key=value"));
  EXPECT_TRUE(URIQuery::Valid("?key=value&key2=value2"));

  EXPECT_TRUE(query.Parse("??"));
  EXPECT_FALSE(query.Parse("invalid?"));
  EXPECT_TRUE(query.Parse("?invalid?"));
  EXPECT_FALSE(query.Parse("? invalid"));
  EXPECT_FALSE(query.Parse("#invalid"));
  EXPECT_FALSE(query.Parse("invalid#"));
  EXPECT_TRUE(query.Parse("?key"));
  EXPECT_TRUE(query.Parse("?key="));
  EXPECT_TRUE(query.Parse("?=value"));
  // these invalid queries failed to parse and
  // didn't update the query.
  // it should still be valid
  EXPECT_TRUE(query.Valid());

  EXPECT_TRUE(query.Parse(""));
  EXPECT_TRUE(query.Parse("?key=value"));
  EXPECT_TRUE(query.Parse("?key=value&key2=value2"));
  // it should still be valid
  EXPECT_TRUE(query.Valid());

  EXPECT_NO_THROW(URIQuery("??"));
  EXPECT_NO_THROW(URIQuery("invalid?"));
  EXPECT_NO_THROW(URIQuery("?invalid?"));
  EXPECT_NO_THROW(URIQuery("? invalid"));
  EXPECT_NO_THROW(URIQuery("#invalid"));
  EXPECT_NO_THROW(URIQuery("invalid#"));
  EXPECT_NO_THROW(URIQuery("?key"));
  EXPECT_NO_THROW(URIQuery("?key="));
  EXPECT_NO_THROW(URIQuery("?=value"));

  EXPECT_NO_THROW(URIQuery(""));
  EXPECT_NO_THROW(URIQuery("?key=value"));
  EXPECT_NO_THROW(URIQuery("?key=value&key2=value2"));
}

/////////////////////////////////////////////////
TEST(URITEST, URIFragment)
{
  URIFragment fragment1, fragment2, fragment3;
  EXPECT_TRUE(fragment1.Str().empty());

  fragment1 = "#f1";
  EXPECT_EQ(fragment1.Str(), "#f1");
  fragment1 = "#f2";
  EXPECT_EQ(fragment1.Str(), "#f2");

  fragment2 = fragment1;
  EXPECT_EQ(fragment2.Str(), fragment1.Str());
  EXPECT_TRUE(fragment2 == fragment1);

  fragment3 = fragment1;
  EXPECT_TRUE(fragment3 == fragment1);

  fragment1.Clear();
  EXPECT_TRUE(fragment1.Str().empty());

  URIFragment fragment4(fragment2);
  EXPECT_TRUE(fragment4 == fragment2);
}

/////////////////////////////////////////////////
TEST(URITEST, URIFragmentCoverageExtra)
{
  // getting full destructor coverage
  URIFragment *p = new URIFragment;
  ASSERT_NE(nullptr, p);
  delete p;
}

/////////////////////////////////////////////////
TEST(URITEST, URIFragmentString)
{
  URIFragment fragment;
  EXPECT_TRUE(fragment.Valid());

  // test static Valid function
  EXPECT_FALSE(URIFragment::Valid("##"));
  EXPECT_FALSE(URIFragment::Valid("invalid#"));
  EXPECT_FALSE(URIFragment::Valid("#invalid#"));
  EXPECT_FALSE(URIFragment::Valid("# invalid"));
  EXPECT_FALSE(URIFragment::Valid("#value with space"));

  EXPECT_TRUE(URIFragment::Valid(""));
  EXPECT_TRUE(URIFragment::Valid("#fragment"));

  // TODO(anyone): uncomment following once URI class is upgraded
  // EXPECT_TRUE(URIFragment::Valid("#fragment/?!$&'()*+,;=:@%20fragment"));

  EXPECT_FALSE(fragment.Parse("##"));
  EXPECT_FALSE(fragment.Parse("invalid#"));
  EXPECT_FALSE(fragment.Parse("#invalid#"));
  EXPECT_FALSE(fragment.Parse("# invalid"));
  EXPECT_FALSE(fragment.Parse("#value with space"));
  // these invalid fragments failed to parse and
  // didn't update the fragment.
  // it should still be valid
  EXPECT_TRUE(fragment.Valid());

  EXPECT_TRUE(fragment.Parse(""));
  EXPECT_TRUE(fragment.Parse("#fragment"));

  // TODO(anyone): uncomment following once URI class is upgraded
  // EXPECT_TRUE(fragment.Parse("#fragment/?!$&'()*+,;=:@%20FRAGMENT"));

  // it should still be valid
  EXPECT_TRUE(fragment.Valid());

  EXPECT_NO_THROW(URIFragment("##"));
  EXPECT_NO_THROW(URIFragment("invalid#"));
  EXPECT_NO_THROW(URIFragment("#invalid#"));
  EXPECT_NO_THROW(URIFragment("# invalid"));
  EXPECT_NO_THROW(URIFragment("#value with space"));

  EXPECT_NO_THROW(URIFragment(""));
  EXPECT_NO_THROW(URIFragment("#fragment"));
  EXPECT_NO_THROW(URIFragment("#fragment/?!$&'()*+,;=:@%20fragment"));
}

/////////////////////////////////////////////////
TEST(URITEST, Scheme)
{
  URI uri;
  EXPECT_TRUE(uri.Str().empty());

  uri.SetScheme("data");
  EXPECT_EQ(uri.Str(), "data://");
  EXPECT_EQ("data", uri.Scheme());
}

/////////////////////////////////////////////////
TEST(URITEST, PathIfHasAuthority)
{
  URI uri;
  EXPECT_FALSE(uri.Authority());
  uri.SetAuthority(URIAuthority());
  EXPECT_TRUE(uri.Authority());

  uri.SetScheme("data");

  uri.Path() = uri.Path() / "world";
  EXPECT_EQ(uri.Str(), "data:world");

  uri.Path() /= "default";
  EXPECT_EQ(uri.Str(), "data:world/default");

  EXPECT_TRUE(uri.Parse("file:///var/run/test"));
  EXPECT_TRUE(uri.Authority());
  EXPECT_EQ(uri.Str(), "file:///var/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file:/var/run/test"));
  EXPECT_TRUE(uri.Authority());
  EXPECT_EQ(uri.Str(), "file:/var/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file://var/run/test"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ(uri.Str(), "file://var/run/test");
  EXPECT_EQ(uri.Authority()->Str(), "//var");
  EXPECT_EQ(uri.Path().Str(), "/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file://test%20space"));
  EXPECT_EQ("//test%20space", uri.Authority()->Str());
  EXPECT_EQ("", uri.Path().Str());

  EXPECT_TRUE(uri.Parse("file:///abs/path/test"));
  EXPECT_EQ(uri.Str(), "file:///abs/path/test");
  EXPECT_EQ(uri.Path().Str(), "/abs/path/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  uri.Parse("file:/var/run/test");
  EXPECT_EQ(uri.Str(), "file:/var/run/test");
  EXPECT_EQ(uri.Path().Str(), "/var/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_FALSE(uri.Parse("file://test+space"));

  EXPECT_TRUE(uri.Parse("file:/test+space"));
  EXPECT_EQ(uri.Str(), "file:/test+space");
  EXPECT_EQ(uri.Path().Str(), "/test+space");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file:/test%20space"));
  EXPECT_EQ(uri.Str(), "file:/test%20space");
  EXPECT_EQ(uri.Path().Str(), "/test%20space");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  uri.Parse("file://C:/Users");
  EXPECT_EQ(uri.Str(), "file:C:/Users");
  EXPECT_TRUE(uri.Path().IsAbsolute());
}

/////////////////////////////////////////////////
TEST(URITEST, Path)
{
  URI uri;
  uri.SetScheme("data");

  uri.Path() = uri.Path() / "world";
  EXPECT_EQ(uri.Str(), "data://world");

  uri.Path() /= "default";
  EXPECT_EQ(uri.Str(), "data://world/default");

  EXPECT_TRUE(uri.Parse("file:///var/run/test"));
  EXPECT_EQ(uri.Str(), "file:///var/run/test");

  EXPECT_TRUE(uri.Parse("file:/var/run/test"));
  EXPECT_EQ(uri.Str(), "file:///var/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file://var/run/test"));
  EXPECT_EQ(uri.Str(), "file://var/run/test");
  EXPECT_FALSE(uri.Authority());
  EXPECT_EQ(uri.Path().Str(), "var/run/test");
  EXPECT_FALSE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file://test%20space"));
  EXPECT_EQ("test%20space", uri.Path().Str());

  EXPECT_TRUE(uri.Parse("file:///abs/path/test"));
  EXPECT_EQ(uri.Str(), "file:///abs/path/test");
  EXPECT_EQ(uri.Path().Str(), "/abs/path/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  uri.Parse("file:/var/run/test");
  EXPECT_EQ(uri.Str(), "file:///var/run/test");
  EXPECT_EQ(uri.Path().Str(), "/var/run/test");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_FALSE(uri.Parse("file://test+space"));

  EXPECT_TRUE(uri.Parse("file:/test+space"));
  EXPECT_EQ(uri.Str(), "file:///test+space");
  EXPECT_EQ(uri.Path().Str(), "/test+space");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  EXPECT_TRUE(uri.Parse("file:/test%20space"));
  EXPECT_EQ(uri.Str(), "file:///test%20space");
  EXPECT_EQ(uri.Path().Str(), "/test%20space");
  EXPECT_TRUE(uri.Path().IsAbsolute());

  uri.Parse("file://C:/Users");
  EXPECT_EQ(uri.Str(), "file://C:/Users");
  EXPECT_TRUE(uri.Path().IsAbsolute());
}

/////////////////////////////////////////////////
TEST(URITEST, PathCopy)
{
  URI uri;
  uri.SetScheme("data");
  uri.Path().PushFront("world");

  const auto uriCopy(uri);
  const auto pathCopy(uriCopy.Path() / "default");

  EXPECT_NE(uri.Path().Str(), pathCopy.Str());
  EXPECT_EQ(uri.Path().Str(), "world");
  EXPECT_EQ(pathCopy.Str(), "world/default");
}

/////////////////////////////////////////////////
TEST(URITEST, Query)
{
  URI uri;
  uri.SetScheme("data");

  uri.Query().Insert("p", "v");
  EXPECT_EQ(uri.Str(), "data://?p=v");

  uri.Path().PushFront("default");
  EXPECT_EQ(uri.Str(), "data://default?p=v");

  uri.Path().PushFront("world");
  EXPECT_EQ(uri.Str(), "data://world/default?p=v");

  URI uri2 = uri;

  uri.Path().Clear();
  EXPECT_EQ(uri.Str(), "data://?p=v");

  uri.Query().Clear();
  EXPECT_EQ(uri.Str(), "data://");

  uri.Clear();
  uri2.Clear();
  EXPECT_EQ(uri, uri2);
}

/////////////////////////////////////////////////
TEST(URITEST, QueryCopy)
{
  URI uri;
  uri.SetScheme("data");
  uri.Query().Insert("a", "b");

  auto uriTmp(uri);
  uriTmp.Query().Insert("c", "d");
  const auto uriCopy(uriTmp);
  const auto queryCopy(uriCopy.Query());

  EXPECT_NE(uri.Query().Str(), queryCopy.Str());
  EXPECT_EQ(uri.Query().Str(), "?a=b");
  EXPECT_EQ(queryCopy.Str(), "?a=b&c=d");
}

/////////////////////////////////////////////////
TEST(URITEST, Fragment)
{
  URI uri;
  uri.SetScheme("data");

  uri.Fragment() = "#f";
  EXPECT_EQ(uri.Str(), "data://#f");

  uri.Path().PushFront("default");
  EXPECT_EQ(uri.Str(), "data://default#f");

  uri.Path().PushFront("world");
  EXPECT_EQ(uri.Str(), "data://world/default#f");

  URI uri2 = uri;

  uri.Path().Clear();
  EXPECT_EQ(uri.Str(), "data://#f");

  uri.Fragment().Clear();
  EXPECT_EQ(uri.Str(), "data://");

  uri.Clear();
  uri2.Clear();
  EXPECT_EQ(uri, uri2);
}

/////////////////////////////////////////////////
TEST(URITEST, FragmentCopy)
{
  URI uri;
  uri.SetScheme("data");
  uri.Fragment() = "#f";

  auto uriTmp(uri);
  uriTmp.Fragment() = "#g";
  const auto uriCopy(uriTmp);
  const auto fragmentCopy(uriCopy.Fragment());

  EXPECT_NE(uri.Fragment().Str(), fragmentCopy.Str());
  EXPECT_EQ(uri.Fragment().Str(), "#f");
  EXPECT_EQ(uriTmp.Str(), "data://#g");
  EXPECT_EQ(uriCopy.Str(), "data://#g");
  EXPECT_EQ(fragmentCopy.Str(), "#g");
}

/////////////////////////////////////////////////
TEST(URITEST, URIString)
{
  URI uri;
  EXPECT_FALSE(URI::Valid(""));
  EXPECT_FALSE(URI::Valid("scheme"));
  EXPECT_FALSE(URI::Valid("scheme://"));
  EXPECT_FALSE(URI::Valid("scheme://?key=value"));
  EXPECT_TRUE(URI::Valid("scheme://part1?keyvalue"));
  EXPECT_FALSE(URI::Valid("scheme://part1?key value"));
  EXPECT_TRUE(URI::Valid("scheme://part1"));
  EXPECT_TRUE(URI::Valid("scheme://part1/part2"));
  EXPECT_TRUE(URI::Valid("scheme://part1?key=value"));
  EXPECT_TRUE(URI::Valid("scheme://part1/part2?k1=v1&k2=v2"));
  EXPECT_FALSE(URI::Valid("scheme://part+1/part+2?k1=v1&k2=v2"));
  EXPECT_TRUE(URI::Valid("scheme://part%201/part%202?k1=v1&k2=v2"));

  EXPECT_TRUE(URI::Valid("scheme://part 1/part 2?k1=v1&k2=v2"));
  EXPECT_TRUE(URI::Valid("scheme://part1 /part2 ?k1=v1&k2=v2"));
  EXPECT_TRUE(URI::Valid("scheme://part  1  /part  2  ?k1=v1&k2=v2"));
  EXPECT_TRUE(URI::Valid("scheme with space://part 1/part 2?k1=v1&k2=v2"));
  // TODO(anyone): switch to the following once URI class is upgraded
  // EXPECT_FALSE(URI::Valid("scheme://part 1/part 2?k1=v1&k2=v2"));
  // EXPECT_FALSE(URI::Valid("scheme://part1 /part2 ?k1=v1&k2=v2"));
  // EXPECT_FALSE(URI::Valid("scheme://part  1  /part  2  ?k1=v1&k2=v2"));
  // EXPECT_FALSE(URI::Valid("scheme with space://part 1/part 2?k1=v1&k2=v2"));

  EXPECT_FALSE(uri.Parse(""));
  EXPECT_FALSE(uri.Parse("scheme"));
  EXPECT_FALSE(uri.Parse("scheme://"));
  EXPECT_FALSE(uri.Parse("scheme://?key=value"));
  EXPECT_TRUE(uri.Parse("scheme://part1?keyvalue"));
  EXPECT_TRUE(uri.Parse("scheme://part1"));
  EXPECT_TRUE(uri.Parse("scheme://part1/part2"));
  EXPECT_TRUE(uri.Parse("scheme://part1?key=value"));
  EXPECT_TRUE(uri.Parse("scheme://part1/part2?k1=v1&k2=v2"));
  EXPECT_FALSE(uri.Parse("scheme://part+1/part+2?k1=v1&k2=v2"));
  EXPECT_TRUE(uri.Parse("scheme://part%201/part%202?k1=v1&k2=v2"));

  EXPECT_NO_THROW(EXPECT_FALSE(URI("").Valid()));
  EXPECT_NO_THROW(EXPECT_FALSE(URI("scheme").Valid()));
  EXPECT_NO_THROW(EXPECT_FALSE(URI("scheme://").Valid()));
  EXPECT_NO_THROW(EXPECT_FALSE(URI("scheme://?key=value").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URI("scheme://part1?keyvalue").Valid()));
  EXPECT_NO_THROW(EXPECT_TRUE(URI("scheme://part1?keyvalue").Valid()));

  EXPECT_NO_THROW(URI("scheme://part1"));
  EXPECT_NO_THROW(URI("scheme://part1/part2"));
  EXPECT_NO_THROW(URI("scheme://part1?key=value"));
  EXPECT_NO_THROW(URI("scheme://part1/part2?k1=v1&k2=v2"));
  EXPECT_NO_THROW(URI("scheme://part+1/part+2?k1=v1&k2=v2"));
  EXPECT_NO_THROW(URI("scheme://part%201/part%202?k1=v1&k2=v2"));
}

/////////////////////////////////////////////////
TEST(URITEST, WikipediaTests)
{
  URI uri;
  uri.SetAuthority(URIAuthority());

  // The following tests were pulled from:
  // https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Examples

  EXPECT_TRUE(uri.Parse("https://john.doe@www.example.com:123/forum/questions"
        "/?tag=networking&order=newest#top"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("https", uri.Scheme());
  EXPECT_EQ("john.doe", uri.Authority()->UserInfo());
  EXPECT_EQ("www.example.com", uri.Authority()->Host());
  EXPECT_EQ(123, *uri.Authority()->Port());
  EXPECT_EQ("//john.doe@www.example.com:123", uri.Authority()->Str());
  EXPECT_EQ("/forum/questions/", uri.Path().Str());
  EXPECT_EQ("?tag=networking&order=newest", uri.Query().Str());
  EXPECT_EQ("#top", uri.Fragment().Str());

  EXPECT_TRUE(uri.Parse("ldap://[2001:db8::7]/c=GB?objectClass?one"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("ldap", uri.Scheme());
  EXPECT_EQ("[2001:db8::7]", uri.Authority()->Host());
  EXPECT_EQ("//[2001:db8::7]", uri.Authority()->Str());
  EXPECT_EQ("/c=GB", uri.Path().Str());
  EXPECT_EQ("?objectClass?one", uri.Query().Str());

  EXPECT_TRUE(uri.Parse("mailto:John.Doe@example.com"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("mailto", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->Str().empty());
  EXPECT_EQ("John.Doe@example.com", uri.Path().Str());
  EXPECT_TRUE(uri.Query().Str().empty());
  EXPECT_TRUE(uri.Fragment().Str().empty());

  EXPECT_TRUE(uri.Parse("news:comp.infosystems.www.servers.unix"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("news", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->Str().empty());
  EXPECT_EQ("comp.infosystems.www.servers.unix", uri.Path().Str());
  EXPECT_TRUE(uri.Query().Str().empty());
  EXPECT_TRUE(uri.Fragment().Str().empty());

  EXPECT_TRUE(uri.Parse("tel:+1-816-555-1212"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("tel", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->Str().empty());
  EXPECT_EQ("+1-816-555-1212", uri.Path().Str());
  EXPECT_TRUE(uri.Query().Str().empty());
  EXPECT_TRUE(uri.Fragment().Str().empty());

  EXPECT_TRUE(uri.Parse("telnet://192.0.2.16:80/"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("telnet", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->UserInfo().empty());
  EXPECT_EQ("192.0.2.16", uri.Authority()->Host());
  EXPECT_EQ(80, uri.Authority()->Port());
  EXPECT_EQ("/", uri.Path().Str());

  EXPECT_TRUE(uri.Parse("urn:oasis:names:specification:docbook:dtd:xml:4.1.2"));
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("urn", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->Str().empty());
  EXPECT_EQ("oasis:names:specification:docbook:dtd:xml:4.1.2",
      uri.Path().Str());
  EXPECT_TRUE(uri.Query().Str().empty());
  EXPECT_TRUE(uri.Fragment().Str().empty());
}

/////////////////////////////////////////////////
TEST(URITEST, URIAuthority)
{
  URIAuthority auth;

  // Empty
  EXPECT_TRUE(auth.UserInfo().empty());
  EXPECT_TRUE(auth.Host().empty());
  EXPECT_FALSE(auth.Port());

  auth.SetUserInfo("bob.smith");
  EXPECT_EQ("bob.smith", auth.UserInfo());
  // No host has been set yet, so the string will be empty.
  EXPECT_EQ("", auth.Str());
  EXPECT_TRUE(auth.Valid());

  auth.SetHost("google.com");
  EXPECT_EQ("google.com", auth.Host());
  EXPECT_EQ("//bob.smith@google.com", auth.Str());
  EXPECT_TRUE(auth.Valid());

  auth.SetPort(1234);
  EXPECT_EQ(1234, *auth.Port());
  EXPECT_EQ("//bob.smith@google.com:1234", auth.Str());
  EXPECT_TRUE(auth.Valid());

  URIAuthority auth2;
  EXPECT_FALSE(auth == auth2);
  auth2 = auth;
  EXPECT_TRUE(auth == auth2);
  EXPECT_TRUE(auth2.Valid());
}

//////////////////////////////////////////////////
TEST(URITEST, File)
{
  URI uri;
  uri.SetAuthority(URIAuthority());
  ASSERT_TRUE(uri.Authority());

  EXPECT_TRUE(uri.Parse("file:relative/path"));
  EXPECT_EQ("relative/path", uri.Path().Str());
  EXPECT_FALSE(uri.Authority()->EmptyHostValid());

  EXPECT_TRUE(uri.Parse("file:/abs/path"));
  EXPECT_EQ("/abs/path", uri.Path().Str());
  EXPECT_FALSE(uri.Authority()->EmptyHostValid());

  // Empty host is valid for file: scheme
  EXPECT_TRUE(uri.Parse("file:///abs/path"));
  EXPECT_EQ("/abs/path", uri.Path().Str());
  EXPECT_TRUE(uri.Authority()->EmptyHostValid());
}

//////////////////////////////////////////////////
TEST(URITEST, WinPath)
{
  // Windows path requires authority
  const auto uri = gz::common::URI("file://D:/my/test/dir/world.sdf",
      true);
  ASSERT_TRUE(uri.Authority());
  EXPECT_EQ("file", uri.Scheme());
  EXPECT_TRUE(uri.Authority()->Str().empty());
  EXPECT_EQ("file:D:/my/test/dir/world.sdf", uri.Str());
}

//////////////////////////////////////////////////
TEST(URITEST, HasAuthority)
{
  {
    // No authority by default
    URI uri("https://john.doe@www.example.com:123/forum/questions/");

    EXPECT_FALSE(uri.Authority());
    EXPECT_EQ("john.doe@www.example.com:123/forum/questions/",
        uri.Path().Str());
    EXPECT_EQ("https://john.doe@www.example.com:123/forum/questions/",
        uri.Str());

    // Modifyng path updates string
    uri.Path() = URIPath("new_authority.com/another/path");

    EXPECT_EQ("new_authority.com/another/path", uri.Path().Str());
    EXPECT_EQ("https://new_authority.com/another/path", uri.Str());

    // Clearing keeps false authority
    uri.Clear();
    EXPECT_EQ("", uri.Str());
    EXPECT_FALSE(uri.Authority());
  }

  {
    // Has authority
    URI uri("https://john.doe@www.example.com:123/forum/questions/", true);

    ASSERT_TRUE(uri.Authority());
    EXPECT_EQ("john.doe", uri.Authority()->UserInfo());
    EXPECT_EQ("www.example.com", uri.Authority()->Host());
    EXPECT_EQ(123, *uri.Authority()->Port());
    EXPECT_EQ("//john.doe@www.example.com:123", uri.Authority()->Str());

    EXPECT_EQ("/forum/questions/", uri.Path().Str());

    uri.SetAuthority(URIAuthority("//new_authority.com"));
    EXPECT_EQ("//new_authority.com", uri.Authority()->Str());
    EXPECT_EQ("/forum/questions/", uri.Path().Str());
    EXPECT_EQ("https://new_authority.com/forum/questions/", uri.Str());

    // Clearing keeps true authority, but empty
    uri.Clear();
    EXPECT_EQ("", uri.Str());
    ASSERT_TRUE(uri.Authority());
    EXPECT_EQ("", uri.Authority()->Str());
  }
}

//////////////////////////////////////////////////
TEST(URITEST, Resource)
{
  // Test URIs that are commonly used for resources in Gazebo
  {
    URI uri;
    EXPECT_TRUE(uri.Parse("model://model_name/meshes/mesh.dae"));
    EXPECT_EQ("model", uri.Scheme());
    EXPECT_FALSE(uri.Authority());
    EXPECT_EQ("model_name/meshes/mesh.dae", uri.Path().Str());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_FALSE(uri.Path().IsAbsolute());
  }

  {
    URI uri;
    uri.SetAuthority(URIAuthority());
    EXPECT_TRUE(uri.Parse("model://model_name/meshes/mesh.dae"));
    EXPECT_EQ("model", uri.Scheme());
    ASSERT_TRUE(uri.Authority());
    EXPECT_EQ("//model_name", uri.Authority()->Str());
    EXPECT_EQ("model_name", uri.Authority()->Host());
    EXPECT_TRUE(uri.Authority()->UserInfo().empty());
    EXPECT_EQ("/meshes/mesh.dae", uri.Path().Str());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_TRUE(uri.Path().IsAbsolute());
  }

  {
    URI uri;
    EXPECT_TRUE(uri.Parse("model://model_name"));
    EXPECT_EQ("model", uri.Scheme());
    EXPECT_FALSE(uri.Authority());
    EXPECT_EQ("model_name", uri.Path().Str());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_FALSE(uri.Path().IsAbsolute());
  }

  {
    URI uri;
    uri.SetAuthority(URIAuthority());
    EXPECT_TRUE(uri.Parse("model://model_name"));
    EXPECT_EQ("model", uri.Scheme());
    ASSERT_TRUE(uri.Authority());
    EXPECT_EQ("//model_name", uri.Authority()->Str());
    EXPECT_EQ("model_name", uri.Authority()->Host());
    EXPECT_TRUE(uri.Authority()->UserInfo().empty());
    EXPECT_TRUE(uri.Path().Str().empty());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_FALSE(uri.Path().IsAbsolute());
  }

  {
    URI uri;
    EXPECT_TRUE(uri.Parse("package://package_name/models/model"));
    EXPECT_EQ("package", uri.Scheme());
    EXPECT_FALSE(uri.Authority());
    EXPECT_EQ("package_name/models/model", uri.Path().Str());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_FALSE(uri.Path().IsAbsolute());
  }

  {
    URI uri;
    uri.SetAuthority(URIAuthority());
    EXPECT_TRUE(uri.Parse("package://package_name/models/model"));
    EXPECT_EQ("package", uri.Scheme());
    ASSERT_TRUE(uri.Authority());
    EXPECT_EQ("//package_name", uri.Authority()->Str());
    EXPECT_EQ("package_name", uri.Authority()->Host());
    EXPECT_TRUE(uri.Authority()->UserInfo().empty());
    EXPECT_EQ("/models/model", uri.Path().Str());
    EXPECT_TRUE(uri.Query().Str().empty());
    EXPECT_TRUE(uri.Fragment().Str().empty());
    EXPECT_TRUE(uri.Path().IsAbsolute());
  }
}
