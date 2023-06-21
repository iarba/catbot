#ifndef DPP_MODULES_MASTER_HPP
#define DPP_MODULES_MASTER_HPP

#include "common.hpp"

class master_t
{
  protected:
  std::string process_help(int argc, std::string *argv, const dpp::message *context_msg);
  std::string process_opensource(int args, std::string *argv, const dpp::message *context_msg);
  std::string process_banana(int args, std::string *argv, const dpp::message *context_msg);
  std::string process_level(int args, std::string *argv, const dpp::message *context_msg);
  std::string process_master(int argc, std::string *argv, const dpp::message *context_msg);
    std::string process_master_level(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_level_exp(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_level_base(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_level_co(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_level_quantum(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_level_mfa(int argc, std::string *argv, const dpp::message *context_msg);
    std::string process_master_ydb(int argc, std::string *argv, const dpp::message *context_msg);
    std::string process_master_allow(int argc, std::string *argv, const dpp::message *context_msg);
    std::string process_master_config(int argc, std::string *argv, const dpp::message *context_msg);
      std::string process_master_config_save(int argc, std::string *argv, const dpp::message *context_msg);
    std::string process_master_prefix(int argc, std::string *argv, const dpp::message *context_msg);
  public:

  std::string process(int argc, std::string *argv, const dpp::message *context_msg);
};

#endif // DPP_MODULES_MASTER_HPP
