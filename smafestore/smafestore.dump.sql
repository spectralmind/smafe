--
-- PostgreSQL database dump
--


SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

--
-- Name: plpgsql; Type: PROCEDURAL LANGUAGE; Schema: -; Owner: postgres
--

CREATE PROCEDURAL LANGUAGE plpgsql;


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: collection; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE collection (
    id integer NOT NULL,
    collection_name character varying NOT NULL
);


ALTER TABLE public.collection OWNER TO smafeadmins;

--
-- Name: collection_file; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE collection_file (
    collection_id integer NOT NULL,
    file_id integer NOT NULL
);


ALTER TABLE public.collection_file OWNER TO smafeadmins;

--
-- Name: distance; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE distance (
    track_a_id integer NOT NULL,
    track_b_id integer NOT NULL,
    featurevectortype_id integer NOT NULL,
    distancetype_id integer NOT NULL,
    value double precision,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone
);


ALTER TABLE public.distance OWNER TO smafeadmins;

--
-- Name: distancejob; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE distancejob (
    featurevectortype_id integer NOT NULL,
    track_id integer NOT NULL,
    distancetype_id integer NOT NULL,
    smafejob_addfile_id integer,
    status character varying,
    priority integer NOT NULL DEFAULT 0,
    created timestamp without time zone NOT NULL DEFAULT now(),
    started timestamp without time zone,
    finished timestamp without time zone
);


ALTER TABLE public.distancejob OWNER TO smafeadmins;

--
-- Name: COLUMN distancejob.status; Type: COMMENT; Schema: public; Owner: smafeadmins
--

COMMENT ON COLUMN distancejob.status IS '<null>: not started
!= <null>: ID of distance calculation daemon that is currently processing this job';


--
-- Name: distancetype; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE distancetype (
    id integer NOT NULL,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone,
    name character varying
);


ALTER TABLE public.distancetype OWNER TO smafeadmins;

--
-- Name: featurevector; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE featurevector (
    track_id integer NOT NULL,
    featurevectortype_id integer NOT NULL,
    data bytea,
    file_id integer,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone
);


ALTER TABLE public.featurevector OWNER TO smafeadmins;

--
-- Name: featurevectorsegment; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

-- do not use bigint as there is no standardized c++ datatype equivalent
CREATE TABLE featurevectorsegment (
    segmentnr integer NOT NULL,
    track_id integer NOT NULL,
    featurevectortype_id integer NOT NULL,
    data bytea,
    file_id integer,
    startsample integer,
    length integer,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone
);


ALTER TABLE public.featurevectorsegment OWNER TO smafeadmins;

--
-- Name: featurevectortype; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE featurevectortype (
    id integer NOT NULL,
    name character varying,
    version integer,
    dimension_x integer,
    dimension_y integer,
    parameters character varying,
    class_id character varying,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone
);


ALTER TABLE public.featurevectortype OWNER TO smafeadmins;

--
-- Name: file; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE file (
    id integer NOT NULL,
    hash character varying,
    track_id integer,
    inserted timestamp(0) without time zone default current_timestamp,
    updated timestamp(0) without time zone,
    input_format character varying,
    uri character varying NOT NULL,
    samplef integer,
    bitrate integer,
    channels integer,
    encoding character varying,
    samplebit integer,
    external_key character varying,
    guid character varying NOT NULL,
    CONSTRAINT file_guid_key UNIQUE (guid),
    CONSTRAINT file_external_key_key UNIQUE (external_key)
);


ALTER TABLE public.file OWNER TO smafeadmins;

--
-- Name: track; Type: TABLE; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE TABLE track (
    id integer NOT NULL,
    fingerprint character varying,
    inserted timestamp(0) without time zone,
    updated timestamp(0) without time zone,
    mbid integer
);


ALTER TABLE public.track OWNER TO smafeadmins;


-- inserted by epei 


-- Table: smafejob_addfile

-- DROP TABLE smafejob_addfile;

CREATE TABLE smafejob_addfile
(
  id serial NOT NULL,
  priority integer NOT NULL DEFAULT 0,
  file_uri character varying NOT NULL,
  created timestamp without time zone NOT NULL DEFAULT now(),
  started timestamp without time zone,
  finished1 timestamp without time zone,
  started2 timestamp without time zone,
  finished2 timestamp without time zone,
  finished timestamp without time zone,
  status character varying,
  collection_name character varying,
  log character varying,
  external_key character varying,
  guid character varying NOT NULL,
  CONSTRAINT smafejob_pkey PRIMARY KEY (id)
)
WITH (OIDS=FALSE);
ALTER TABLE smafejob_addfile OWNER TO smafeadmins;
GRANT ALL ON TABLE smafejob_addfile TO smafeadmins;
GRANT SELECT, UPDATE, INSERT ON TABLE smafejob_addfile TO smafeusers;


-- Table: smafejob_deletecollection

-- DROP TABLE smafejob_deletecollection;

CREATE TABLE smafejob_deletecollection
(
  id serial NOT NULL,
  priority integer NOT NULL DEFAULT 0,
  collection_name character varying NOT NULL,
  created timestamp without time zone NOT NULL DEFAULT now(),
  started timestamp without time zone,
  finished1 timestamp without time zone,
  started2 timestamp without time zone,
  finished2 timestamp without time zone,
  finished timestamp without time zone,
  status character varying,
  log character varying,
  CONSTRAINT smafejob_deletecollection_pkey PRIMARY KEY (id)
)
WITH (OIDS=FALSE);
ALTER TABLE smafejob_deletecollection OWNER TO smafeadmins;
GRANT SELECT, UPDATE, INSERT ON TABLE smafejob_deletecollection TO smafeusers;



-- Table: smafejob_deletefile

-- DROP TABLE smafejob_deletefile;

CREATE TABLE smafejob_deletefile
(
  id serial NOT NULL,
  priority integer NOT NULL DEFAULT 0,
  collection_name character varying,
  file_id integer  NOT NULL,
  created timestamp without time zone NOT NULL DEFAULT now(),
  started timestamp without time zone,
  finished1 timestamp without time zone,
  started2 timestamp without time zone,
  finished2 timestamp without time zone,
  finished timestamp without time zone,
  status character varying,
  log character varying,
  CONSTRAINT smafejob_deletefile_pkey PRIMARY KEY (id)
)
WITH (OIDS=FALSE);
ALTER TABLE smafejob_deletefile OWNER TO smafeadmins;
GRANT SELECT, UPDATE, INSERT ON TABLE smafejob_deletefile TO smafeusers;

















--
-- Name: create_distance_indexes(); Type: FUNCTION; Schema: public; Owner: smafeadmins
--

CREATE FUNCTION create_distance_indexes() RETURNS void
    AS $$CREATE INDEX distance_traid_fvtid_distid_value_idx ON distance USING btree (track_a_id, featurevectortype_id, distancetype_id, value);
    ALTER TABLE distance
  		ADD CONSTRAINT distance_pkey PRIMARY KEY(distancetype_id, track_b_id, featurevectortype_id, track_a_id);
  $$
    LANGUAGE sql;


ALTER FUNCTION public.create_distance_indexes() OWNER TO smafeadmins;

--
-- Name: drop_distance_indexes(); Type: FUNCTION; Schema: public; Owner: smafeadmins
--

CREATE FUNCTION drop_distance_indexes() RETURNS void
    AS $$drop INDEX distance_traid_fvtid_distid_value_idx;
    ALTER TABLE distance DROP CONSTRAINT distance_pkey;$$
    LANGUAGE sql;


ALTER FUNCTION public.drop_distance_indexes() OWNER TO smafeadmins;

--
-- Name: distance_indexes_exist(); Type: FUNCTION; Schema: public; Owner: smafeadmins
--

CREATE FUNCTION distance_indexes_exist() RETURNS boolean
    AS $$DECLARE
    num_idx integer;
BEGIN
	select into num_idx count(*) from pg_indexes WHERE tablename='distance' AND indexname = 'distance_traid_fvtid_distid_value_idx' OR indexname = 'distance_pkey';

	if num_idx = 2 then
    	RETURN true;
	else
		RETURN false;
	END if;

END;

$$
    LANGUAGE plpgsql;


ALTER FUNCTION public.distance_indexes_exist() OWNER TO smafeadmins;


-- end - inserted by epei


--
-- Name: seq_collection; Type: SEQUENCE; Schema: public; Owner: smafeadmins
--

CREATE SEQUENCE seq_collection
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.seq_collection OWNER TO smafeadmins;

--
-- Name: seq_collection; Type: SEQUENCE OWNED BY; Schema: public; Owner: smafeadmins
--

ALTER SEQUENCE seq_collection OWNED BY collection.id;


--
-- Name: seq_distancetype; Type: SEQUENCE; Schema: public; Owner: smafeadmins
--

CREATE SEQUENCE seq_distancetype
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1
    CYCLE;


ALTER TABLE public.seq_distancetype OWNER TO smafeadmins;

--
-- Name: seq_distancetype; Type: SEQUENCE OWNED BY; Schema: public; Owner: smafeadmins
--

ALTER SEQUENCE seq_distancetype OWNED BY distancetype.id;


--
-- Name: seq_featurevectortype; Type: SEQUENCE; Schema: public; Owner: smafeadmins
--

CREATE SEQUENCE seq_featurevectortype
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1
    CYCLE;


ALTER TABLE public.seq_featurevectortype OWNER TO smafeadmins;

--
-- Name: seq_featurevectortype; Type: SEQUENCE OWNED BY; Schema: public; Owner: smafeadmins
--

ALTER SEQUENCE seq_featurevectortype OWNED BY featurevectortype.id;


--
-- Name: seq_file; Type: SEQUENCE; Schema: public; Owner: smafeadmins
--

CREATE SEQUENCE seq_file
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1
    CYCLE;


ALTER TABLE public.seq_file OWNER TO smafeadmins;

--
-- Name: seq_file; Type: SEQUENCE OWNED BY; Schema: public; Owner: smafeadmins
--

ALTER SEQUENCE seq_file OWNED BY file.id;


--
-- Name: seq_track; Type: SEQUENCE; Schema: public; Owner: smafeadmins
--

CREATE SEQUENCE seq_track
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1
    CYCLE;


ALTER TABLE public.seq_track OWNER TO smafeadmins;

--
-- Name: seq_track; Type: SEQUENCE OWNED BY; Schema: public; Owner: smafeadmins
--

ALTER SEQUENCE seq_track OWNED BY track.id;


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: smafeadmins
--

ALTER TABLE collection ALTER COLUMN id SET DEFAULT nextval('seq_collection'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: smafeadmins
--

ALTER TABLE featurevectortype ALTER COLUMN id SET DEFAULT nextval('seq_featurevectortype'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: smafeadmins
--

ALTER TABLE file ALTER COLUMN id SET DEFAULT nextval('seq_file'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: smafeadmins
--

ALTER TABLE track ALTER COLUMN id SET DEFAULT nextval('seq_track'::regclass);


--
-- Name: collection_file_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY collection_file
    ADD CONSTRAINT collection_file_pkey PRIMARY KEY (collection_id, file_id);


--
-- Name: collection_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY collection
    ADD CONSTRAINT collection_pkey PRIMARY KEY (id);

    
 -- Constraint: collection_collection_name_key

-- ALTER TABLE collection DROP CONSTRAINT collection_collection_name_key;

ALTER TABLE collection
  ADD CONSTRAINT collection_collection_name_key UNIQUE(collection_name);


--
-- Name: distance_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY distance
    ADD CONSTRAINT distance_pkey PRIMARY KEY (distancetype_id, track_b_id, featurevectortype_id, track_a_id);


--
-- Name: distancejobtable_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY distancejob
    ADD CONSTRAINT distancejobtable_pkey PRIMARY KEY (featurevectortype_id, track_id, distancetype_id);

ALTER TABLE ONLY distancejob
    ADD CONSTRAINT distancejobtable_fk FOREIGN KEY (smafejob_addfile_id) REFERENCES smafejob_addfile(id) ON UPDATE CASCADE ON DELETE RESTRICT;


    
    
-- added by epei / wj
CREATE INDEX distancejobtable_fvtid_distid_traid_status
  ON distancejob
  USING btree
  (featurevectortype_id, distancetype_id, track_id, status);
    

--
-- Name: distancetype_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY distancetype
    ADD CONSTRAINT distancetype_pkey PRIMARY KEY (id);


--
-- Name: featurevector_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY featurevector
    ADD CONSTRAINT featurevector_pkey PRIMARY KEY (featurevectortype_id, track_id);


--
-- Name: featurevectorsegment_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY featurevectorsegment
    ADD CONSTRAINT featurevectorsegment_pkey PRIMARY KEY (featurevectortype_id, track_id, segmentnr);


--
-- Name: featurevectortype_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY featurevectortype
    ADD CONSTRAINT featurevectortype_pkey PRIMARY KEY (id);


--
-- Name: file_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY file
    ADD CONSTRAINT file_pkey PRIMARY KEY (id);
    
    
  
--added by epei
CREATE INDEX file_track_id_idx
  ON file
  (track_id);
  
  
CREATE INDEX file_external_key_idx
   ON file
   USING btree
   (external_key);


--
-- Name: track_mbid_key; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY track
    ADD CONSTRAINT track_mbid_key UNIQUE (mbid);


--
-- Name: track_pkey; Type: CONSTRAINT; Schema: public; Owner: smafeadmins; Tablespace: 
--

ALTER TABLE ONLY track
    ADD CONSTRAINT track_pkey PRIMARY KEY (id);


--
-- Name: featurevector_track_id_featurevectortype_id_key; Type: INDEX; Schema: public; Owner: smafeadmins; Tablespace: 
--

CREATE UNIQUE INDEX featurevector_track_id_featurevectortype_id_key ON featurevector USING btree (track_id, featurevectortype_id);



-- Name: collection_file_collection_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY collection_file
    ADD CONSTRAINT collection_file_collection_id_fkey FOREIGN KEY (collection_id) REFERENCES collection(id) ON UPDATE CASCADE;


--
-- Name: collection_file_file_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY collection_file
    ADD CONSTRAINT collection_file_file_id_fkey FOREIGN KEY (file_id) REFERENCES file(id) ON UPDATE CASCADE;


--
-- Name: distance_fk1; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY distance
    ADD CONSTRAINT distance_fk1 FOREIGN KEY (distancetype_id) REFERENCES distancetype(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: distance_fk2; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY distance
    ADD CONSTRAINT distance_fk2 FOREIGN KEY (track_a_id, featurevectortype_id) REFERENCES featurevector(track_id, featurevectortype_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: distance_fk3; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY distance
    ADD CONSTRAINT distance_fk3 FOREIGN KEY (track_b_id, featurevectortype_id) REFERENCES featurevector(track_id, featurevectortype_id) ON UPDATE CASCADE ON DELETE RESTRICT;



--
-- Name: distance_traid_fvtid_distid_value_idx; Type: INDEX; Schema: public; Owner: smafeadmins
--

CREATE INDEX distance_traid_fvtid_distid_value_idx
    ON distance (track_a_id, featurevectortype_id, distancetype_id, "value");


--
-- Name: distancejob_fk; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY distancejob
    ADD CONSTRAINT distancejob_fk FOREIGN KEY (track_id, featurevectortype_id) REFERENCES featurevector(track_id, featurevectortype_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: distancejob_fk1; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY distancejob
    ADD CONSTRAINT distancejob_fk1 FOREIGN KEY (distancetype_id) REFERENCES distancetype(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: featurevector_fk1; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY featurevector
    ADD CONSTRAINT featurevector_fk1 FOREIGN KEY (featurevectortype_id) REFERENCES featurevectortype(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: featurevector_fk2; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY featurevector
    ADD CONSTRAINT featurevector_fk2 FOREIGN KEY (track_id) REFERENCES track(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: featurevector_fk3; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY featurevector
    ADD CONSTRAINT featurevector_fk3 FOREIGN KEY (file_id) REFERENCES file(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: featurevectorsegment_fk1; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY featurevectorsegment
    ADD CONSTRAINT featurevectorsegment_fk1 FOREIGN KEY (track_id, featurevectortype_id) REFERENCES featurevector(track_id, featurevectortype_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: featurevectorsegment_fk2; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY featurevectorsegment
    ADD CONSTRAINT featurevectorsegment_fk2 FOREIGN KEY (file_id) REFERENCES file(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: file_fk1; Type: FK CONSTRAINT; Schema: public; Owner: smafeadmins
--

ALTER TABLE ONLY file
    ADD CONSTRAINT file_fk1 FOREIGN KEY (track_id) REFERENCES track(id) ON UPDATE CASCADE ON DELETE RESTRICT;

    
    
    
-- the following two constrraints are already in the create table statement
-- done epei 2010-09-22
-- Constraint: file_external_key_key

-- ALTER TABLE file DROP CONSTRAINT file_external_key_key;

--ALTER TABLE file
--  ADD CONSTRAINT file_external_key_key UNIQUE(external_key);

    
-- Constraint: file_guid_key

-- ALTER TABLE file DROP CONSTRAINT file_guid_key;

--ALTER TABLE file
--  ADD CONSTRAINT file_guid_key UNIQUE(guid);










-- added by epei
-- Table: config

-- DROP TABLE config;

CREATE TABLE config
(
  "key" character varying NOT NULL,
  "value" character varying,
  modified timestamp without time zone DEFAULT now(),
  CONSTRAINT pk_config PRIMARY KEY (key)
)
WITH (OIDS=FALSE);
ALTER TABLE config OWNER TO smafeadmins;
GRANT SELECT, UPDATE, INSERT ON TABLE config TO smafeusers;


-- Function: update_modified_column()

-- DROP FUNCTION update_modified_column();

CREATE OR REPLACE FUNCTION update_modified_column()
  RETURNS trigger AS
$BODY$
	BEGIN
	   NEW.modified = now(); 
	   RETURN NEW;
	END;
	$BODY$
  LANGUAGE 'plpgsql' VOLATILE
  COST 100;
ALTER FUNCTION update_modified_column() OWNER TO smafeadmins;


-- Trigger: update_config_modtime on config

-- DROP TRIGGER update_config_modtime ON config;

CREATE TRIGGER update_config_modtime
  BEFORE UPDATE
  ON config
  FOR EACH ROW
  EXECUTE PROCEDURE update_modified_column();






--
-- Name: collection; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE collection FROM PUBLIC;
REVOKE ALL ON TABLE collection FROM smafeadmins;
GRANT ALL ON TABLE collection TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE collection TO smafeusers;


--
-- Name: collection_file; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE collection_file FROM PUBLIC;
REVOKE ALL ON TABLE collection_file FROM smafeadmins;
GRANT ALL ON TABLE collection_file TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE collection_file TO smafeusers;


--
-- Name: distance; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE distance FROM PUBLIC;
REVOKE ALL ON TABLE distance FROM smafeadmins;
GRANT ALL ON TABLE distance TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE distance TO smafeusers;


--
-- Name: distancejob; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE distancejob FROM PUBLIC;
REVOKE ALL ON TABLE distancejob FROM smafeadmins;
GRANT ALL ON TABLE distancejob TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE distancejob TO smafeusers;


--
-- Name: distancetype; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE distancetype FROM PUBLIC;
REVOKE ALL ON TABLE distancetype FROM smafeadmins;
GRANT ALL ON TABLE distancetype TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE distancetype TO smafeusers;


--
-- Name: featurevector; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE featurevector FROM PUBLIC;
REVOKE ALL ON TABLE featurevector FROM smafeadmins;
GRANT ALL ON TABLE featurevector TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE featurevector TO smafeusers;


--
-- Name: featurevectorsegment; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE featurevectorsegment FROM PUBLIC;
REVOKE ALL ON TABLE featurevectorsegment FROM smafeadmins;
GRANT ALL ON TABLE featurevectorsegment TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE featurevectorsegment TO smafeusers;


--
-- Name: featurevectortype; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE featurevectortype FROM PUBLIC;
REVOKE ALL ON TABLE featurevectortype FROM smafeadmins;
GRANT ALL ON TABLE featurevectortype TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE featurevectortype TO smafeusers;


--
-- Name: file; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE file FROM PUBLIC;
REVOKE ALL ON TABLE file FROM smafeadmins;
GRANT ALL ON TABLE file TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE file TO smafeusers;


--
-- Name: track; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON TABLE track FROM PUBLIC;
REVOKE ALL ON TABLE track FROM smafeadmins;
GRANT ALL ON TABLE track TO smafeadmins;
GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE track TO smafeusers;


--
-- Name: seq_collection; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON SEQUENCE seq_collection FROM PUBLIC;
REVOKE ALL ON SEQUENCE seq_collection FROM smafeadmins;
GRANT ALL ON SEQUENCE seq_collection TO smafeadmins;
GRANT ALL ON SEQUENCE seq_collection TO smafeusers;


--
-- Name: seq_distancetype; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON SEQUENCE seq_distancetype FROM PUBLIC;
REVOKE ALL ON SEQUENCE seq_distancetype FROM smafeadmins;
GRANT ALL ON SEQUENCE seq_distancetype TO smafeadmins;
GRANT ALL ON SEQUENCE seq_distancetype TO smafeusers;


--
-- Name: seq_featurevectortype; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON SEQUENCE seq_featurevectortype FROM PUBLIC;
REVOKE ALL ON SEQUENCE seq_featurevectortype FROM smafeadmins;
GRANT ALL ON SEQUENCE seq_featurevectortype TO smafeadmins;
GRANT ALL ON SEQUENCE seq_featurevectortype TO smafeusers;


--
-- Name: seq_file; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON SEQUENCE seq_file FROM PUBLIC;
REVOKE ALL ON SEQUENCE seq_file FROM smafeadmins;
GRANT ALL ON SEQUENCE seq_file TO smafeadmins;
GRANT ALL ON SEQUENCE seq_file TO smafeusers;


--
-- Name: seq_track; Type: ACL; Schema: public; Owner: smafeadmins
--

REVOKE ALL ON SEQUENCE seq_track FROM PUBLIC;
REVOKE ALL ON SEQUENCE seq_track FROM smafeadmins;
GRANT ALL ON SEQUENCE seq_track TO smafeadmins;
GRANT ALL ON SEQUENCE seq_track TO smafeusers;



--
-- manually added by epei: for generated sequences
--
REVOKE ALL ON SEQUENCE smafejob_addfile_id_seq FROM PUBLIC;
REVOKE ALL ON SEQUENCE smafejob_addfile_id_seq FROM smafeadmins;
GRANT ALL ON SEQUENCE smafejob_addfile_id_seq TO smafeadmins;
GRANT ALL ON SEQUENCE smafejob_addfile_id_seq TO smafeusers;

REVOKE ALL ON SEQUENCE smafejob_deletecollection_id_seq FROM PUBLIC;
REVOKE ALL ON SEQUENCE smafejob_deletecollection_id_seq FROM smafeadmins;
GRANT ALL ON SEQUENCE smafejob_deletecollection_id_seq TO smafeadmins;
GRANT ALL ON SEQUENCE smafejob_deletecollection_id_seq TO smafeusers;

REVOKE ALL ON SEQUENCE smafejob_deletefile_id_seq FROM PUBLIC;
REVOKE ALL ON SEQUENCE smafejob_deletefile_id_seq FROM smafeadmins;
GRANT ALL ON SEQUENCE smafejob_deletefile_id_seq TO smafeadmins;
GRANT ALL ON SEQUENCE smafejob_deletefile_id_seq TO smafeusers;

--
-- PostgreSQL database dump complete
--

