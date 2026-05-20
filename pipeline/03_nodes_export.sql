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


