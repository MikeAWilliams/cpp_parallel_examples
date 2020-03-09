// modified from examples in C++ Consurrency In Action Second Edition, Anthony Williams
// I found out that my compiler doesn't support the parallel algorithms out of the box so I never linked this.
// I plan to come back to it later after I have an easier compiler, or take the time
#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <numeric>
#include <execution>

struct log_info {
    std::string page;
    char first;
    char second;
};

log_info parse_log_line(std::string const &line)
{
    return {line, line[0], line[1]};
}

using visit_map_type= std::unordered_map<std::string, unsigned long long>;

visit_map_type
count_visits_per_page(std::vector<std::string> const &log_lines) {

    struct combine_visits {
        visit_map_type
        operator()(visit_map_type lhs, visit_map_type rhs) const {
            if(lhs.size() < rhs.size())
                std::swap(lhs, rhs);
            for(auto const &entry : rhs) {
                lhs[entry.first]+= entry.second;
            }
            return lhs;
        }

        visit_map_type operator()(log_info log, visit_map_type map) const {
            ++map[log.page];
            return map;
        }
        visit_map_type operator()(visit_map_type map, log_info log) const {
            ++map[log.page];
            return map;
        }
        visit_map_type operator()(log_info log1, log_info log2) const {
            visit_map_type map;
            ++map[log1.page];
            ++map[log2.page];
            return map;
        }
    };

    return std::transform_reduce(
        std::execution::par, log_lines.begin(), log_lines.end(),
        visit_map_type(), combine_visits(), parse_log_line);
}

TEST_CASE("Run It")
{
    auto result{count_visits_per_page({"12", "34", "56", "78", "910"})};
    REQUIRE(false);   
}

