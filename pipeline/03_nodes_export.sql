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
	s.parent_station,
	tmp.route_id,
	tmp.direction_id
from
	tmp
join raw_gtfs.stops s on
	s.stop_id = tmp.stop_id );

drop view if exists metro.nodes_view ;

create view metro.nodes_view as
(
select
	n.node_id,
	n.stop_name,
	n.stop_id,
	n.stop_lat,
	n.stop_lon,
	n.parent_staion,
	r.route_long_name,
	n.direction_id
from
	metro.nodes n
join raw_gtfs.routes r on
	n.route_id = r.route_id
order by
	n.node_id
);

WITH LookupArray AS (
    SELECT FORMAT(
        $$        {"%s", "%s", %s, %s, "%s"}$$, -- $$ acts as a raw string boundary
        stop_id,
        REPLACE(stop_name, '"', '\"'), -- Escapes double quotes for C++
        stop_lat,
        stop_lon,
        REPLACE(route_long_name, '"', '\"')
    ) AS cpp_line
    FROM metro.nodes_view
    ORDER BY node_id
)
--Ghép toàn bộ thành 1 file C++ hoàn chỉnh
SELECT FORMAT(
'#pragma once
#include <array>
#include <string_view>
#include <cstddef> // For std::size_t

namespace GTFSData {
	
	constexpr std::size_t NUM_NODES {%s};
 
    struct Node {
        std::string_view stop_id;
        std::string_view stop_name;
		double lat;
		double lon;
		std::string_view route;
    };

    // 1. Lookup thông tin trạm
    constexpr std::array<Node, NUM_NODES> nodes = {{
%s
    }};

}', 
    (SELECT COUNT(*) FROM metro.nodes_view), -- Điền size mảng 
    (SELECT STRING_AGG(cpp_line, ',' || CHR(10)) FROM LookupArray) -- Nội dung mảng 
);
