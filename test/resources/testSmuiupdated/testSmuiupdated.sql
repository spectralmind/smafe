--- Manually crafted by EP with greatest consideration ---
START TRANSACTION;

CREATE TABLE bubbles1 ( 
  id integer NOT NULL, 
  size integer NOT NULL DEFAULT 1, 
  count integer NOT NULL DEFAULT 1,  
  info character varying, 
  CONSTRAINT primkey_bubbles1 PRIMARY KEY (id) ) WITH (OIDS=FALSE); 
ALTER TABLE bubbles1 OWNER TO smafeadmin; 
GRANT ALL ON TABLE bubbles1 TO smafeadmin; 
GRANT SELECT, UPDATE, INSERT, DELETE ON TABLE bubbles1 TO smafeuser; 
SELECT AddGeometryColumn('', 'bubbles1','geom',4326,'POINT',2); 
ALTER TABLE track ADD COLUMN bubbles1_id integer; 
ALTER TABLE track  ADD CONSTRAINT fk_bubbles1 FOREIGN KEY (bubbles1_id) REFERENCES bubbles1 (id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE NO ACTION;   
INSERT INTO bubbles1 (id,size,count,info,geom) VALUES (1,3000,30,'testing',ST_SetSRID(ST_MakePoint(0,0),4326));
INSERT INTO bubbles1 (id,size,count,info,geom) VALUES (2,1000,10,'testing',ST_SetSRID(ST_MakePoint(3,3),4326));
INSERT INTO bubbles1 (id,size,count,info,geom) VALUES (3,500,5,'testing',ST_SetSRID(ST_MakePoint(10,0),4326));
INSERT INTO bubbles1 (id,size,count,info,geom) VALUES (4,500,5,'testing',ST_SetSRID(ST_MakePoint(12,2),4326));
INSERT INTO bubbles1 (id,size,count,info,geom) VALUES (5,500,5,'testing',ST_SetSRID(ST_MakePoint(14,4),4326));


CREATE TABLE bubbles2 ( 
  id integer NOT NULL, 
  size integer NOT NULL DEFAULT 1, 
  count integer NOT NULL DEFAULT 1,  
  info character varying, 
  CONSTRAINT primkey_bubbles2 PRIMARY KEY (id) ) WITH (OIDS=FALSE); 
ALTER TABLE bubbles2 OWNER TO smafeadmin; 
GRANT ALL ON TABLE bubbles2 TO smafeadmin; 
GRANT SELECT, UPDATE, INSERT, DELETE ON TABLE bubbles2 TO smafeuser; 
SELECT AddGeometryColumn('', 'bubbles2','geom',4326,'POINT',2); 
ALTER TABLE track ADD COLUMN bubbles2_id integer; 
ALTER TABLE track  ADD CONSTRAINT fk_bubbles2 FOREIGN KEY (bubbles2_id) REFERENCES bubbles2 (id) MATCH SIMPLE ON UPDATE NO ACTION ON DELETE NO ACTION;   
INSERT INTO bubbles2 (id,size,count,info,geom) VALUES (1,5000,50,'testing',ST_SetSRID(ST_MakePoint(2,2),4326));
INSERT INTO bubbles2 (id,size,count,info,geom) VALUES (2,1500,15,'testing',ST_SetSRID(ST_MakePoint(12,2),4326));



-- add featurevectortype and dbinfo
-- (would be added by config / or smafewrapd)
-- dummy 3 dim numeric feature vector
-- is encrypted with supersecret
INSERT INTO featurevectortype ("name", "version", dimension_x, dimension_y, parameters) VALUES ('yNnO0yPlehVFPTKx90WllepuGcOK17L0', 1, 3, 1, 'yNnO0yPlehVFPTKx90WllepuGcOK17L0');
-- 20 x 5 SOM with 3 layers (tracks, bubbles1 and ..2)
INSERT INTO dbinfo(numberoflayers, bubblesagginfo, labelsagginfo, dimx, dimy) VALUES (3, 'manually', 'NA', 20, 5);


-- add track
-- (would have been added by an smafejob_addfile)
-- track with id 1
INSERT INTO track (fingerprint, inserted, updated, mbid, geom, bubbles1_id, bubbles2_id) VALUES ('testing', NULL, NULL, NULL, NULL, NULL, NULL);

-- add file
-- (would have been added by an smafejob_addfile)
-- file with id 1, linked to track 1, URI is testing
INSERT INTO file (hash, track_id, inserted, updated, input_format, uri, samplef, bitrate, channels, encoding, samplebit, external_key, guid, songname, album_id, artist_id, recordlabel_id, genre_id, release_year, audio_uri, cover_uri, buy_uri) VALUES ('testing', 1, '2010-04-01 14:31:53', NULL, NULL, 'testing', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
-- file with id 2, linked to track 1, URI is testing2
INSERT INTO file (hash, track_id, inserted, updated, input_format, uri, samplef, bitrate, channels, encoding, samplebit, external_key, guid, songname, album_id, artist_id, recordlabel_id, genre_id, release_year, audio_uri, cover_uri, buy_uri) VALUES ('testing', 1, '2010-04-01 15:31:53', NULL, NULL, 'testing2', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

-- add collection connections
INSERT INTO collection_file(collection_id, file_id) VALUES (2, 1);
INSERT INTO collection_file(collection_id, file_id) VALUES (2, 2);


-- add featurevector
-- (would have been added by an smafejob_addfile)
-- 3 dimensional feature vector, linked to the existing records
INSERT INTO featurevector (track_id, featurevectortype_id, data, file_id)  VALUES (1, 1, 'b1yZ2ecjSLZ2fCO6UQDOaQ/EjXJsw4lX1fJutlBFntlUrWIig0NrBxf0ZJAQeYnY5kMNqTQe
32I4IaRpAlWNfjXfrQAjaCEWpdKia/RkNyKdIdkWb76oIN2z04H6ndzS', 1);

-- add jobs
-- (would have been added by an smafejob_addfile)
-- job says: include track 1 into smui
INSERT INTO smuijob_addtrack(priority, track_id) VALUES (0, 1);




COMMIT;
