


COPY smafejob_addfile (id, priority, file_uri, created, started, finished1, started2, finished2, finished, status, collection_name, log, external_key, guid) FROM stdin;
96	0	/home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3	2011-02-07 12:34:49.926031	2011-02-07 12:35:02.063532	2011-02-07 12:35:02.175923	\N	\N	2011-02-07 12:35:02.175923	OK	\N	-------------------------------\nsmafewrapd, pid=23789\n-------------------------------\n2011-02-07 12:28:56 ii Starting addfile task: id=96, file_uri=/home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3, external_key=\n2011-02-07 12:28:56 ii mp3 file format assumed as file ends in .mp3\n2011-02-07 12:28:57 ii mixing stereo wave to mono\n2011-02-07 12:28:57 ii Writing track record to DB\n2011-02-07 12:28:57 ii Writing file record to DB\n2011-02-07 12:28:57 ii Writing feature vector record(s) to DB\n2011-02-07 12:28:58 ii File moved: /home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3 to /tmp/ABBA_-_SOS_cut.mp3\n	\N	/home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3
\.


COPY track (id, fingerprint, inserted, updated, mbid) FROM stdin;
96	886d2e8cdc22058cfd6a1aa8a4817f1d	\N	\N	\N
\.



COPY file (id, hash, track_id, inserted, updated, input_format, uri, samplef, bitrate, channels, encoding, samplebit, external_key, guid) FROM stdin;
96	886d2e8cdc22058cfd6a1aa8a4817f1d	96	2011-02-07 12:35:02	\N	samplefrequency: 44100, channels: 2, bits/sample: 16, bitrate: 192, encoding: MPEG layer 3 v1, cbr	/home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3	44100	192	2	MPEG layer 3 v1, cbr	16	\N	/home/ewald/code/smafe-trunk/test/resources/testDistd_extended/ABBA_-_SOS_cut.mp3
\.




COPY featurevector (track_id, featurevectortype_id, data, file_id, inserted, updated) FROM stdin;
96	1	ahlXRhlxQmt+uLFYoRdVil8Cb/titTuEcpXyEkHWvP//k1uEEvRqjRBi2BmsyRgA/0kYm17T\\0126tMtFLCl+03Q+GLskPSCtNu5M9EmYOYhUZkYfJA41Y1FFsda6q7TAbQYA4nTtx1HtKfowgUC\\012bDnbTmd8aCFdmzdsFXBZLY96aECXhOWOHnhKJWf27Q5xv5Rx8W+T0AddlqQR4QTxDGGPT7Yg\\012R5y7ktulKpir0eHLdgb4fNRk/oxA1uQ/tKIRfvZloFBhHosI0musibZ5OJlG0JMUt9lLHh+Z\\012zXYAChpPNBkC/Kn+co6CLuNlDgOOChGCm5kMrDV5qPQgCqE41kIXPxrVWK6UePU/w98KEJgN\\0129W6KhsyUXkVEPozcLlvxjWvL9TkXVm+4AGrXL+syjKul0IKBFCK4ylkGRtRR+P9JOj2jRQb9\\012zv2ZvR7b4wtxzFwVLCJ/i0Kx44iUHQYSMEe8Wb2FWA4DVwF/w+mGtEgjn9ZNGVVXRONhMrSA\\012V33aN7B6elNmExETomdC8L9SorzBgZ7Tw3zTLAaeUvj1CXCGfqTp999jhtcd/0/MFghRxV2p\\012GcX2nNgGf+FOJeORzEjxDCW875qO/lR3jflOAWN8c/Zz6BDKJg0URX1yzy7qsFi3PWo8wI5C\\012b+vmHETEB95UzM5+ql+skFV46yn21GInErrM/MoSzY9jr94NBIooTiWnfzAydBfGCs7tMy3U\\0122/o2lo5fYvPCZ2sa5Dft6UN5R5wHJzXctFBKmI7iiX3tjjODxeD18MoZY2WsO3hhxfF8yfQK\\012veskKyyDE4mY98yy0qEssqisM+XpU9k4BwaHn938W5X3TN624f5SmupZqPo7PgQZ/Q78hRuo\\012Lk/sHiovGSYvzy3uakxtUJ74htk+GtdVp/OBrvWTZWceHmz5i4NVqvlR1WD8LzUkx/DlD70w\\012JwS2IR5FWTwJEpTQbwJLr0loEvbK8dAjax1hyu7uaRxftKboHb8+BbgGiaCblKr26KS49oGr\\012cl9eiJ4nsNBTjqctifd1pvACVCYt//fMbIS5KAAA2vwZmuX6fOtvQd95Fyf6aLg0OyK7FOVs\\012wegSwzAkPQy5yBH10nOaJXbBFGYleKO70zMGluRabOBrUHG94vv3u4Lva5pOpfDJbqOtyIXZ\\012GFxWdMweHinkWfF5VtDxwgFty48PiLmsGOKPvjBn6S6HrYCZCeBf7GEZF/FHabj8mLZx04MI\\012WZ78redAGNtUlLO1RieWfU1wGbYdgQW5JTU+HjtG54KgAOhBXODsIRjMN36nxTWX8b2x5Ymr\\012y6DHuxic9uSZ7jo1DYIEHAaDltOaTani3iCXsEb+fAq7uAKFmoG7OzxXgFNScjt93Ik9yJsR\\0123WrBvoPjcKjy9XYQkdVKBukoM5NPFX/rM+9i1lpjcT0gGOKsC6TvK8el0cRm/4cuzK2HjgD7\\012wCEDh2lJdMOeSe8kxDP88apDLRTymIasd08U9Iuvf8pM5ZVgzisOz3pYnyHWLNTjfDGbPeg+\\012flKBHGO92UjPFGgYidevo0Kudr14AI4QcLh0GoYSgU6BKTb0B7c1mQnrbsKnYaS+OYJ4Ci61\\012FMG8QtWQ1I6uGDBFaWAYJaMmzksPWFjg3z0m5fILPgrTqqR3\\012	96	\N	\N
\.





COPY distancejob (featurevectortype_id, track_id, distancetype_id, smafejob_addfile_id, status, priority, created, started, finished) FROM stdin;
1	96	1	96	\N	0	2011-02-07 12:35:02.175923	\N	\N
1	96	2	96	\N	0	2011-02-07 12:35:02.175923	\N	\N
1	96	3	96	\N	0	2011-02-07 12:35:02.175923	\N	\N
1	96	4	96	\N	0	2011-02-07 12:35:02.175923	\N	\N
1	96	5	96	\N	0	2011-02-07 12:35:02.175923	\N	\N
\.


COPY collection_file (collection_id, file_id) FROM stdin;
2	96
\.




