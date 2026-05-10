begin;

set search_path to raw_gtfs;

delete
from
	stop_times as st
where
	not exists (
	select
		1
	from
		routes as r,
		trips as t
	where
		t.route_id = r.route_id
		and st.trip_id = t.trip_id
		and r.route_type = 1);

delete
from
	trips as t
where
	not exists (
	select
		1
	from
		routes as r
	where
		t.route_id = r.route_id
		and r.route_type = 1);

delete
from
	routes
where
	route_type != 1;

delete
from
	agency as a
where
	not exists (
	select 1
	from
		routes as r
	where
		r.agency_id = a.agency_id);

delete
from
	calendar_dates cd
where
	not exists (
	select
		1
	from
		trips t
	where
		t.service_id = cd.service_id);

delete
from
	calendar as c
where
	not exists (
	select 1
	from
		trips as t
	where
		t.service_id = c.service_id);

delete
from
	shapes as s
where
	not exists (
	select
		1
	from
		trips as t
	where
		s.shape_id = t.shape_id);

delete
from
	shape_id_lookup as sil
where
	not exists (
	select
		1
	from
		trips as t
	where
		sil.shape_id = t.shape_id);

with tmp_stop_id as (
select
distinct stop_id
from
stop_times )
delete
from
transfers as t
where
t.from_stop_id not in (
select
*
from
tmp_stop_id)
or t.to_stop_id not in (
select
*
from
tmp_stop_id);


alter table stop_times drop constraint if exists stop_times_stop_id_fkey;



create table metro_stop_id as (
select
	distinct stop_id
from
	stop_times);



create table metro_parent_station_id as (
select
	distinct parent_station
from
	stops as s
join metro_stop_id as msi on
	msi.stop_id = s.stop_id and s.parent_station is not null);


with tmp as (
select
	*
from
	metro_stop_id
union
select
	*
from
	metro_parent_station_id)
delete
from
	stops s
where
	s.stop_id not in (select * from tmp);

drop table if exists metro_parent_station_id;

drop table if exists metro_stop_id;

alter table stop_times
add foreign key (stop_id) references stops(stop_id);

commit;

