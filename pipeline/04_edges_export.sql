create materialized view metro.TravelEdgeBase as (
	select
		st.trip_id,
		t.route_id,
		st.stop_id as source_stop,
		lead(st.stop_id) over (partition by st.trip_id order by st.stop_sequence) as target_stop,
		st.arrival_time as source_time,
		lead(st.arrival_time) over (partition by st.trip_id order by st.stop_sequence) as target_time
	from
	raw_gtfs.stop_times st join raw_gtfs.trips t on st.trip_id = t.trip_id  
);


create view metro.TravelRawEdges as (
	select
		source_stop,
		target_stop,
		route_id,
		extract(EPOCH from (target_time::interval - source_time::interval)) as travel_time_seconds
	from
		metro.TravelEdgeBase
	where
		target_stop is not null
);

create view metro.TravelEdges as(
select
	n1.node_id as source_node,
	n2.node_id as target_node,
	-- Calculate the median (50th percentile) travel time to ignore outliers
	PERCENTILE_CONT(0.5) within group (order by travel_time_seconds) as travel_time_seconds
from
	metro.TravelRawEdges
join metro.nodes n1 on
	TravelRawEdges.source_stop = n1.stop_id
	and TravelRawEdges.route_id = n1.route_id
join metro.nodes n2 on
	TravelRawEdges.target_stop = n2.stop_id
	and TravelRawEdges.route_id = n2.route_id
group by
	source_node,
	target_node
);

create materialized view metro.NodeHeadways as (
    with PlatformArrivals as (
		select
			st.stop_id,
			t.route_id,
			st.arrival_time as current_train_time,
			lead(st.arrival_time) over (partition by st.stop_id, t.route_id order by st.arrival_time) as next_train_time
		from
			raw_gtfs.stop_times st
		join raw_gtfs.trips t on
			st.trip_id = t.trip_id  
    ),
    HeadwaySeconds as (
		select
			stop_id,
			route_id,
			extract(EPOCH from (next_train_time::interval - current_train_time::interval)) as headway
		from
			PlatformArrivals
		where
			next_train_time is not null
    )
-- Calculate Expected Wait Time (Median Headway / 2)
	select
		n.node_id,
		(PERCENTILE_CONT(0.5) within group (
		order by headway)) / 2 as expected_wait_seconds
	from
		HeadwaySeconds hs
	join metro.nodes n on
		hs.stop_id = n.stop_id
	and hs.route_id = n.route_id
where
	headway > 0
	-- This ignores the duplicate calendar times found!
group by
	n.node_id
);


create view metro.TransferEdges as(
select
	n1.node_id as source_node,
	n2.node_id as target_node,
	t.min_transfer_time + hw.expected_wait_seconds as transfer_time
from
	raw_gtfs.transfers t
join metro.nodes n1 on
	t.from_stop_id = n1.stop_id
join metro.nodes n2 on
	t.to_stop_id = n2.stop_id
join metro.nodeheadways hw on
	n2.node_id = hw.node_id
where
	n1.node_id != n2.node_id
and (t.transfer_type is not null or t.transfer_type != '3'));

create table metro.Edges as (
(
	select
		*
	from
		metro.TransferEdges t)
union all
(
	select
		*
	from
		metro.TravelEdges t) 
);


