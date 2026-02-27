#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "atm.hpp"
#include "catch.hpp"

////////////////////////////////////////////////////////////////////////////////
// Helper: CompareFiles
////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) return false;

  std::string a, b;
  while (f1 >> a && f2 >> b) {
    if (a != b) return false;
  }

  return f1.eof() && f2.eof();
}

////////////////////////////////////////////////////////////////////////////////
// Sanity Tests (Provided Examples)
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("CreateAccount_Basic", "[sanity]") {
  Atm atm;
  auto key = std::make_pair(12345678u, 1234u);

  atm.RegisterAccount(12345678u, 1234u, "Sam Sepiol", 300.30);

  auto& accounts = atm.GetAccounts();
  REQUIRE(accounts.contains(key));
  REQUIRE(accounts.size() == 1);

  const Account& acc = accounts.at(key);
  REQUIRE(acc.owner_name == "Sam Sepiol");
  REQUIRE(acc.balance == Approx(300.30));

  auto& tx = atm.GetTransactions();
  REQUIRE(tx.contains(key));
  REQUIRE(tx.at(key).empty());
}

TEST_CASE("Withdraw_Basic", "[sanity]") {
  Atm atm;
  auto key = std::make_pair(1u, 2u);

  atm.RegisterAccount(1u, 2u, "A", 100.0);
  atm.WithdrawCash(1u, 2u, 20.0);

  const Account& acc = atm.GetAccounts().at(key);
  REQUIRE(acc.balance == Approx(80.0));
}

////////////////////////////////////////////////////////////////////////////////
// Critical Error Tests (Bug Hunting)
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Register_Duplicate_Throws", "[bug-register]") {
  Atm atm;
  atm.RegisterAccount(1u, 2u, "Alice", 100.0);

  REQUIRE_THROWS_AS(atm.RegisterAccount(1u, 2u, "Bob", 200.0),
                    std::invalid_argument);
}

TEST_CASE("Withdraw_Negative_Throws", "[bug-withdraw-neg]") {
  Atm atm;
  atm.RegisterAccount(1u, 2u, "Alice", 100.0);

  REQUIRE_THROWS_AS(atm.WithdrawCash(1u, 2u, -1.0), std::invalid_argument);
}

TEST_CASE("Withdraw_Overdraft_Throws", "[bug-withdraw-over]") {
  Atm atm;
  atm.RegisterAccount(1u, 2u, "Alice", 50.0);

  REQUIRE_THROWS_AS(atm.WithdrawCash(1u, 2u, 60.0), std::runtime_error);
}

TEST_CASE("Deposit_Negative_Throws", "[bug-deposit-neg]") {
  Atm atm;
  atm.RegisterAccount(1u, 2u, "Alice", 50.0);

  REQUIRE_THROWS_AS(atm.DepositCash(1u, 2u, -5.0), std::invalid_argument);
}

TEST_CASE("Withdraw_Invalid_Account_Throws", "[bug-invalid-withdraw]") {
  Atm atm;

  REQUIRE_THROWS_AS(atm.WithdrawCash(999u, 888u, 10.0), std::invalid_argument);
}

TEST_CASE("Deposit_Invalid_Account_Throws", "[bug-invalid-deposit]") {
  Atm atm;

  REQUIRE_THROWS_AS(atm.DepositCash(999u, 888u, 10.0), std::invalid_argument);
}

TEST_CASE("PrintLedger_Invalid_Account_Throws", "[bug-invalid-print]") {
  Atm atm;

  REQUIRE_THROWS_AS(atm.PrintLedger("out.txt", 999u, 888u),
                    std::invalid_argument);
}

TEST_CASE("Transactions_Record_Created", "[bug-transaction-record]") {
  Atm atm;
  auto key = std::make_pair(10u, 20u);

  atm.RegisterAccount(10u, 20u, "User", 100.0);

  auto& tx = atm.GetTransactions();
  REQUIRE(tx.contains(key));
  REQUIRE(tx.at(key).empty());

  atm.DepositCash(10u, 20u, 10.0);
  REQUIRE(tx.at(key).size() == 1);

  atm.WithdrawCash(10u, 20u, 5.0);
  REQUIRE(tx.at(key).size() == 2);

  REQUIRE(tx.at(key)[0].find("Deposit") != std::string::npos);
  REQUIRE(tx.at(key)[1].find("Withdrawal") != std::string::npos);
}

TEST_CASE("Deposit_Updates_Balance", "[critical]") {
  Atm atm;
  const auto key = std::make_pair(11111111u, 2222u);

  atm.RegisterAccount(11111111u, 2222u, "Alice", 10.00);

  atm.DepositCash(11111111u, 2222u, 25.50);

  auto& accounts = atm.GetAccounts();
  REQUIRE(accounts.contains(key));
  REQUIRE(accounts.at(key).balance == Approx(35.50));
}