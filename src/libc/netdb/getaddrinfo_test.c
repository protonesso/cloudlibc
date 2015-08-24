// Copyright (c) 2015 Nuxi, https://nuxi.nl/
//
// This file is distrbuted under a 2-clause BSD license.
// See the LICENSE file for details.

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stddef.h>
#include <testing.h>

// Macros for testing calls to getaddrinfo() that return entries.
#define TEST_AI_BEGIN(nodename, servname, flags, family, socktype, protocol) \
  do {                                                                       \
    struct addrinfo *res;                                                    \
    {                                                                        \
      struct addrinfo hints = {.ai_flags = AI_NUMERICHOST | (flags),         \
                               .ai_family = (family),                        \
                               .ai_socktype = (socktype),                    \
                               .ai_protocol = (protocol)};                   \
      ASSERT_EQ(0, getaddrinfo(nodename, servname, &hints, &res));           \
    }                                                                        \
    struct addrinfo *ai = res;
#define TEST_AI_ENTRY(node, service, socktype, protocol)                      \
  do {                                                                        \
    /* Validate basic entry properties. */                                    \
    ASSERT_NE(NULL, ai);                                                      \
    ASSERT_EQ(ai->ai_addr->sa_family, ai->ai_family);                         \
    ASSERT_EQ(socktype, ai->ai_socktype);                                     \
    ASSERT_EQ(protocol, ai->ai_protocol);                                     \
                                                                              \
    char nodebuf[sizeof(node)];                                               \
    char servicebuf[sizeof(#service)];                                        \
    ASSERT_EQ(0, getnameinfo(ai->ai_addr, ai->ai_addrlen, nodebuf,            \
                             sizeof(nodebuf), servicebuf, sizeof(servicebuf), \
                             NI_NUMERICHOST | NI_NUMERICSERV));               \
    ASSERT_STREQ(node, nodebuf);                                              \
    ASSERT_STREQ(#service, servicebuf);                                       \
                                                                              \
    ai = ai->ai_next;                                                         \
  } while (0)
#define TEST_AI_END()  \
  ASSERT_EQ(NULL, ai); \
  freeaddrinfo(res);   \
  }                    \
  while (0)

// Tests a failing call to getaddrinfo().
#define TEST_AI_FAIL(nodename, servname, flags, family, socktype, protocol, \
                     error)                                                 \
  do {                                                                      \
    struct addrinfo *res;                                                   \
    struct addrinfo hints = {.ai_flags = AI_NUMERICHOST | (flags),          \
                             .ai_family = (family),                         \
                             .ai_socktype = (socktype),                     \
                             .ai_protocol = (protocol)};                    \
    ASSERT_EQ(error, getaddrinfo(nodename, servname, &hints, &res));        \
  } while (0)

TEST(getaddrinfo, bad) {
  // Nothing to resolve.
  TEST_AI_FAIL(NULL, NULL, 0, AF_UNSPEC, 0, 0, EAI_NONAME);
  // Bad flags.
  TEST_AI_FAIL("127.0.0.1", "80", 0xdeadc0de, AF_UNSPEC, 0, 0, EAI_BADFLAGS);
  // Bad address family.
  TEST_AI_FAIL("127.0.0.1", "80", 0, 0xdeadc0de, 0, 0, EAI_FAMILY);
  // Unknown service name.
  TEST_AI_FAIL("127.0.0.1", "foobarbaz", 0, AF_UNSPEC, 0, 0, EAI_NONAME);
  // Service name has to be a string.
  TEST_AI_FAIL(NULL, "who", AI_NUMERICSERV, AF_UNSPEC, SOCK_DGRAM, 0,
               EAI_NONAME);
  // Bad socket type.
  TEST_AI_FAIL("127.0.0.1", "http", 0, AF_INET, 0xdeadc0de, 0, EAI_SOCKTYPE);
  // Hostname resolution is not available.
  TEST_AI_FAIL("example.com", "80", 0, AF_UNSPEC, 0, 0, EAI_NONAME);
  // Wrong address family for node name.
  TEST_AI_FAIL("::1", "80", 0, AF_INET, 0, 0, EAI_NONAME);
  TEST_AI_FAIL("127.0.0.1", "80", 0, AF_INET6, 0, 0, EAI_NONAME);
}

TEST(getaddrinfo, any) {
  TEST_AI_BEGIN(NULL, "who", AI_PASSIVE, AF_UNSPEC, SOCK_DGRAM, 0);
  TEST_AI_ENTRY("::", 513, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("0.0.0.0", 513, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();

  TEST_AI_BEGIN(NULL, "111", AI_PASSIVE | AI_NUMERICSERV, AF_INET, SOCK_STREAM,
                0);
  // TODO(ed): Should this return IPPROTO_TCP instead?
  TEST_AI_ENTRY("0.0.0.0", 111, SOCK_STREAM, 0);
  TEST_AI_END();

  TEST_AI_BEGIN(NULL, "snmp", AI_PASSIVE, AF_INET6, 0, 0);
  TEST_AI_ENTRY("::", 161, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::", 161, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();

  // The "http-alt" service has three different port numbers listed.
  TEST_AI_BEGIN(NULL, "http-alt", AI_PASSIVE, AF_UNSPEC, 0, 0);
  TEST_AI_ENTRY("::", 591, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::", 591, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("::", 8008, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::", 8008, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("::", 8080, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::", 8080, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("0.0.0.0", 591, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("0.0.0.0", 591, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("0.0.0.0", 8008, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("0.0.0.0", 8008, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("0.0.0.0", 8080, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("0.0.0.0", 8080, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();
}

TEST(getaddrinfo, localhost) {
  TEST_AI_BEGIN(NULL, "www", 0, AF_UNSPEC, 0, 0);
  TEST_AI_ENTRY("::1", 80, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::1", 80, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("127.0.0.1", 80, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("127.0.0.1", 80, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();

  TEST_AI_BEGIN(NULL, "smtp", 0, AF_INET, 0, 0);
  TEST_AI_ENTRY("127.0.0.1", 25, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("127.0.0.1", 25, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();

  TEST_AI_BEGIN(NULL, "domain", 0, AF_INET6, 0, 0);
  TEST_AI_ENTRY("::1", 53, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("::1", 53, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();
}

TEST(getaddrinfo, inet) {
  TEST_AI_BEGIN("10.0.0.1", NULL, 0, AF_UNSPEC, SOCK_STREAM, 0);
  TEST_AI_ENTRY("10.0.0.1", 0, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_END();

  TEST_AI_BEGIN("10.0.0.1", "pop3", 0, AF_UNSPEC, SOCK_STREAM, 0);
  TEST_AI_ENTRY("10.0.0.1", 110, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_END();

  TEST_AI_BEGIN("10.0.0.1", "smtp", 0, AF_INET, 0, 0);
  TEST_AI_ENTRY("10.0.0.1", 25, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_ENTRY("10.0.0.1", 25, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_END();

  // The "whosockami" service has two distinct ports for TCP and UDP. We
  // should return both correctly if no socket type is set.
  TEST_AI_BEGIN("255.255.255.255", "whosockami", 0, AF_INET, 0, 0);
  TEST_AI_ENTRY("255.255.255.255", 2009, SOCK_DGRAM, IPPROTO_UDP);
  TEST_AI_ENTRY("255.255.255.255", 2019, SOCK_STREAM, IPPROTO_TCP);
  TEST_AI_END();

  // TODO(ed): Support full dotted notation.
}

// TODO(ed): Add tests for IPv6.
