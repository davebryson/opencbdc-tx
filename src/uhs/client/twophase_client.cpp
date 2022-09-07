// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
//               2022 MITRE Corporation
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "twophase_client.hpp"

#include "uhs/transaction/messages.hpp"

namespace cbdc {
    twophase_client::twophase_client(
        const cbdc::config::options& opts,
        const std::shared_ptr<logging::log>& logger,
        const std::string& wallet_file,
        const std::string& client_file)
        : client(opts, logger, wallet_file, client_file),
          m_coordinator_client(opts.m_coordinator_endpoints[0]),
          m_shard_status_client(opts.m_locking_shard_readonly_endpoints,
                                opts.m_shard_ranges,
                                m_client_timeout),
          m_logger(logger),
          m_opts(opts) {}

    auto twophase_client::init_derived() -> bool {
        if(!m_coordinator_client.init()) {
            m_logger->error("Failed to initialize coordinator client");
            return false;
        }

        if(!m_shard_status_client.init()) {
            m_logger->error("Failed to initialize shard status client");
            return false;
        }

        return true;
    }

    auto twophase_client::sync() -> bool {
        auto success = true;

        auto txids = std::set<hash_t>();
        for(const auto& [tx_id, tx] : pending_txs()) {
            txids.insert(tx_id);
        }
        for(const auto& [tx_id, inp] : pending_inputs()) {
            txids.insert(tx_id);
        }

        for(const auto& tx_id : txids) {
            m_logger->debug("Requesting status of", to_string(tx_id));
            auto res = m_shard_status_client.check_tx_id(tx_id);
            if(!res.has_value()) {
                m_logger->error("Timeout waiting for shard response");
                success = false;
            } else {
                if(res.value()) {
                    m_logger->info(to_string(tx_id), "confirmed");
                    confirm_transaction(tx_id);
                } else {
                    m_logger->info(to_string(tx_id), "not found");
                }
            }
        }

        return success;
    }

    auto twophase_client::check_tx_id(const hash_t& tx_id)
        -> std::optional<bool> {
        return m_shard_status_client.check_tx_id(tx_id);
    }

    auto twophase_client::check_unspent(const hash_t& uhs_id)
        -> std::optional<bool> {
        return m_shard_status_client.check_unspent(uhs_id);
    }

}
