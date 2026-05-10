begin;

drop schema if exists raw_gtfs cascade; 

create schema raw_gtfs;

set search_path to raw_gtfs;

create table raw_gtfs.agency(
agency_id varchar primary key,
agency_name varchar not null,
agency_url varchar not null,
agency_timezone varchar not null,
agency_fare_url varchar
);

create table raw_gtfs.stops(
stop_id varchar primary key,
stop_code varchar,
stop_name varchar not null,
stop_lat double precision not null,
stop_lon double precision not null,
wheelchair_boarding smallint,
stop_timezone varchar,
location_type smallint,
parent_station varchar references raw_gtfs.stops(stop_id),
level_id varchar,
constraint check_wheelchair_boarding check  (wheelchair_boarding in (0,1,2)),
constraint check_location_type check (location_type in (0,1,2,3,4))
);

create table raw_gtfs.routes(
route_id varchar primary key,
agency_id varchar references agency(agency_id),
route_short_name varchar not null,
route_long_name varchar,
route_type smallint not null,
route_color varchar,
route_text_color varchar
);

create table raw_gtfs.calendar(
service_id varchar primary key,
monday smallint,
tuesday smallint,
wednesday smallint,
thursday smallint,
friday smallint,
saturday smallint,
sunday smallint,
start_date date,
end_date date
);

create table raw_gtfs.calendar_dates(
service_id varchar not null references calendar(service_id),
date date not null,
exception_type smallint not null,
constraint check_exception_type check(exception_type in (1,2))
);

create table raw_gtfs.shape_id_lookup(
shape_id varchar primary key
);

create table raw_gtfs.shapes(
shape_id varchar not null references shape_id_lookup,
shape_pt_lat double precision not null,
shape_pt_lon double precision not null,
shape_pt_sequence smallint not null,
primary key (shape_id, shape_pt_sequence),
constraint check_shape_pt_sequence check(shape_pt_sequence >= 0)
);


create table raw_gtfs.trips(
route_id varchar not null references routes(route_id),
service_id varchar not null references calendar(service_id),
trip_id varchar primary key,
trip_headsign varchar,
trip_short_name varchar,
direction_id smallint,
shape_id varchar references shape_id_lookup,
wheelchair_accessible smallint ,
bikes_allowed smallint,
constraint check_bikes_allowed check (bikes_allowed in (0,1,2))
);


create table raw_gtfs.stop_times(
trip_id varchar not null references trips(trip_id),
arrival_time varchar,
departure_time varchar,
stop_id varchar references stops(stop_id),
stop_sequence smallint not null,
stop_headsign varchar,
pickup_type smallint,
drop_off_type smallint,
timepoint smallint,
primary key (trip_id, stop_sequence),
constraint check_stop_sequence check (stop_sequence >= 0),
constraint check_pickup_type check (pickup_type in (0,1,2,3)),
constraint check_drop_off_type check (drop_off_type in (0,1,2,3))
);

create table raw_gtfs.transfers(
from_stop_id varchar not null references stops(stop_id),
to_stop_id varchar not null references stops(stop_id),
transfer_type smallint not null,
min_transfer_time int,
primary key  (from_stop_id, to_stop_id),
constraint check_transfer_type check (transfer_type in (0,1,2,3))
);



copy raw_gtfs.agency
from
 '/container_data/gtfs/agency.txt' 
    with (format csv, header match,
delimiter ',' ) ;


copy raw_gtfs.stops
from
 '/container_data/gtfs/stops.txt' 
    with (format csv, header match, delimiter ',', null '' ) ;

copy raw_gtfs.routes
from
 '/container_data/gtfs/routes.txt' 
    with (format csv, header match, delimiter ',', null '' ) ;

copy raw_gtfs.calendar
from
 '/container_data/gtfs/calendar.txt' 
    with (format csv, header match, delimiter ',', null '' ) ;

copy raw_gtfs.calendar_dates
from
 '/container_data/gtfs/calendar_dates.txt' 
    with (format csv, header match, delimiter ',', null '' ) ;

alter table raw_gtfs.shapes drop constraint shapes_shape_id_fkey;

copy raw_gtfs.shapes
from
 '/container_data/gtfs/shapes.txt'
    with (format csv, header match, delimiter ',', null '' ) ;

insert
	into
	raw_gtfs.shape_id_lookup
	(
	select
		distinct shape_id
	from
		raw_gtfs.shapes);

alter table raw_gtfs.shapes add foreign key (shape_id) references raw_gtfs.shape_id_lookup;

copy raw_gtfs.trips
from
 '/container_data/gtfs/trips.txt'
    with (format csv, header match, delimiter ',', null '' ) ;

copy raw_gtfs.stop_times
from
 '/container_data/gtfs/stop_times.txt'
    with (format csv, header match, delimiter ',', null '' ) ;

copy raw_gtfs.transfers
from
 '/container_data/gtfs/transfers.txt'
    with (format csv, header match, delimiter ',', null '' ) ;

commit;

