--- Manually crafted by EP with greatest consideration ---
START TRANSACTION;

-- add job
-- (would have been added by smintapi or stuff (from external)
-- job says: delete file 1
INSERT INTO smafejob_deletefile(priority, collection_name, file_id) VALUES (0, '', 1);

COMMIT;
