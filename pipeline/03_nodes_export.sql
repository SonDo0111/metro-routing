drop schema if exists metro cascade;

create schema metro;

drop table if exists metro.nodes ;

create table metro.nodes as(
with tmp as (
select
	distinct st.stop_id,
	t.route_id,
	t.direction_id
from
	raw_gtfs.stop_times st
join raw_gtfs.trips t on
	st.trip_id = t.trip_id
order by route_id, direction_id
)
select
	(row_number() over (order by tmp.route_id, tmp.direction_id, tmp.stop_id)) - 1 as node_id,
    s.stop_name,
	s.stop_id,
	s.stop_lat,
	s.stop_lon,
	tmp.route_id,
	tmp.direction_id
from
	tmp
join raw_gtfs.stops s on
	s.stop_id = tmp.stop_id );


WITH 
-- 1. Mảng xuôi (Index -> String): Sắp xếp theo node_id
ForwardArray AS (
    SELECT '        "' || stop_id || '"' AS cpp_line
    FROM metro.nodes
    ORDER BY node_id
),
-- 2. Mảng ngược (String -> Index): Sắp xếp theo stop_id (Alphabet) để Binary Search
ReverseArray AS (
    SELECT '        {"' || stop_id || '", ' || node_id || '}' AS cpp_line
    FROM metro.nodes
    ORDER BY stop_id
)
-- 3. Ghép toàn bộ thành 1 file C++ hoàn chỉnh
SELECT FORMAT(
'#pragma once
#include <array>
#include <string_view>
#include <algorithm>

namespace GTFSData {
    // 1. Lookup xuôi: O(1)
    constexpr std::array<std::string_view, %s> int_to_gtfs = {
%s
    };

    struct Node {
        std::string_view stop_id;
        int node_id;
    };

    // 2. Lookup ngược: Đã được DB sắp xếp sẵn A-Z
    constexpr std::array<Node, %s> gtfs_to_int = {{
%s
    }};

    // 3. Hàm tra cứu nhị phân: O(log N)
    inline int get_node_id(std::string_view target_id) {
        auto it = std::lower_bound(gtfs_to_int.begin(), gtfs_to_int.end(), target_id,
            [](const Node& node, std::string_view stop_id) { 
                return item.stop_id < val; 
            });
            
        if (it != gtfs_to_int.end() && it->stop_id == target_id) {
            return it->node_id;
        }
        return -1; // Không tìm thấy
    }
}', 
    (SELECT COUNT(*) FROM metro.nodes), -- Điền size mảng 1
    (SELECT STRING_AGG(cpp_line, ',' || CHR(10)) FROM ForwardArray), -- Nội dung mảng 1
    (SELECT COUNT(*) FROM metro.nodes), -- Điền size mảng 2
    (SELECT STRING_AGG(cpp_line, ',' || CHR(10)) FROM ReverseArray)  -- Nội dung mảng 2
);

