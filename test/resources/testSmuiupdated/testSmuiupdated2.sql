--- Manually crafted by EP with greatest consideration ---
START TRANSACTION;

-- add jobs
-- (would have been added by an smafejob_deletefile)
-- job says: delete track 1 
INSERT INTO smuijob_deletetrack(priority, track_id) VALUES (0, 1);

COMMIT;
