

--
-- Data for Name: collection; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

-- in std dump
--COPY collection (id, collection_name) FROM stdin;
--1	_r
--2	_d
--\.


-- insert  additional collection
COPY collection (id, collection_name) FROM stdin;
3	testcollection
\.



--
-- Data for Name: track; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY track (id, fingerprint, inserted, updated, mbid) FROM stdin;
1	7791189f9ef82b6cbdeb83059c60ae31	\N	\N	\N
2	203fe3071adc816a6f3575458270823a	\N	\N	\N
\.



--
-- Data for Name: file; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY file (id, hash, track_id, inserted, updated, input_format, uri, samplef, bitrate, channels, encoding, samplebit, external_key, guid) FROM stdin;
1	7791189f9ef82b6cbdeb83059c60ae31	1	2011-12-14 12:05:40	\N	samplefrequency: 44100, channels: 2, bits/sample: 16, bitrate: 40, #samples: 1160687l, encoding: MPEG layer 3 v1, cbr	/home/ewald/code/smafe-trunk/test/resources/testsmall.mp3	44100	40	2	MPEG layer 3 v1, cbr	16	\N	/home/ewald/code/smafe-trunk/test/resources/testsmall.mp3
2	7791189f9ef82b6cbdeb83059c60ae31	1	2011-12-14 12:05:41	\N	samplefrequency: 44100, channels: 2, bits/sample: 16, bitrate: 40, #samples: 1160687l, encoding: MPEG layer 3 v1, cbr	/home/ewald/code/smafe-trunk/test/resources/testthird.mp3	44100	40	2	MPEG layer 3 v1, cbr	16	\N	/home/ewald/code/smafe-trunk/test/resources/testthird.mp3
3	203fe3071adc816a6f3575458270823a	2	2011-12-14 12:05:42	\N	samplefrequency: 44100, channels: 2, bits/sample: 16, bitrate: 40, #samples: 1057007l, encoding: MPEG layer 3 v1, cbr	/home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3	44100	40	2	MPEG layer 3 v1, cbr	16	\N	/home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3
\.

--
-- Data for Name: collection_file; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

-- epei: added references from track 2 (=file 3) to collection 3
COPY collection_file (collection_id, file_id) FROM stdin;
2	1
2	2
2	3
3	3
\.



--
-- Data for Name: config; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY config (key, value, modified) FROM stdin;
an3dcVSeXlULn8umQWmkV6sEsbh1t2lTIGJi0Y8l6ZM=\n	an3dcVSeXlULn8umQWmkVynsdzQI08AROida85a2r9I=\n	2011-12-14 11:57:59.66927
o9urbIyZ2sxq1ce8CJGOKDGX7nGKE/2HiYR6iOLkXXU=\n	o9urbIyZ2sxq1ce8CJGOKFB1age8lC0z\n	2011-12-14 11:57:59.696358
o9urbIyZ2sxq1ce8CJGOKMOGln6rwnvCAgnu4MKDpQs=\n	o9urbIyZ2sxq1ce8CJGOKFB1age8lC0z\n	2011-12-14 11:57:59.721963
ZeOmBDcYx8qJu7t3g+wRKhfMwPoYCjyp5ix+nh98nJ0=\n	ZeOmBDcYx8qJu7t3g+wRKjHqUk8Ttg0e\n	2011-12-14 11:57:59.749078
ZeOmBDcYx8qJu7t3g+wRKspAQUk7yhg6/JMwQ9Yb7n46ulbsqIT0iQ==\n	ZeOmBDcYx8qJu7t3g+wRKiTopIxZTSqu\n	2011-12-14 11:57:59.78137
ZeOmBDcYx8qJu7t3g+wRKpDcxltn9yJnjab3Z8HjEtaLoda+e3xTAA==\n	ZeOmBDcYx8qJu7t3g+wRKpjG8aEFWeeX\n	2011-12-14 11:57:59.806495
ZeOmBDcYx8qJu7t3g+wRKlxxwL6JXPqPp8oZEGMeNB6fkLMnIbfB7w==\n	ZeOmBDcYx8qJu7t3g+wRKiTopIxZTSqu\n	2011-12-14 11:57:59.834082
ZeOmBDcYx8qJu7t3g+wRKlxxwL6JXPqP835pqx+AO2I=\n	ZeOmBDcYx8qJu7t3g+wRKiTopIxZTSqu\n	2011-12-14 11:57:59.876045
ZeOmBDcYx8qJu7t3g+wRKmQphS2ZTmW2p9y8OULgZAY=\n	ZeOmBDcYx8qJu7t3g+wRKqI2dRo5xwD0\n	2011-12-14 11:57:59.902574
ZeOmBDcYx8qJu7t3g+wRKqzVyduHpojZhzNrA4X8pUo=\n	ZeOmBDcYx8qJu7t3g+wRKpjG8aEFWeeX\n	2011-12-14 11:57:59.93218
ZeOmBDcYx8qJu7t3g+wRKgDKhC1PBvnAqTcrpjtDJJ4=\n	ZeOmBDcYx8qJu7t3g+wRKkuZBhdmRxhM\n	2011-12-14 11:57:59.958272
ZeOmBDcYx8qJu7t3g+wRKp5FfPLwkSLWpmLeEi/ILerwhl21PbWM1sQQ0p8YiGuR\n	ZeOmBDcYx8qJu7t3g+wRKiTopIxZTSqu\n	2011-12-14 11:58:00.0695
ZeOmBDcYx8qJu7t3g+wRKrUVs2VxvXf/Qh6uRM5D0LQ=\n	ZeOmBDcYx8qJu7t3g+wRKpjG8aEFWeeX\n	2011-12-14 11:58:00.109796
ZeOmBDcYx8qJu7t3g+wRKrUVs2VxvXf/4KdQYbhLRoM=\n	ZeOmBDcYx8qJu7t3g+wRKpjG8aEFWeeX\n	2011-12-14 11:58:00.134924
ZeOmBDcYx8qJu7t3g+wRKrcPD64KaKuJ60XDAmnRtq8=\n	ZeOmBDcYx8qJu7t3g+wRKlZxFA8XJQMF\n	2011-12-14 11:58:00.193846
ZeOmBDcYx8qJu7t3g+wRKuPjhQrwk/pCd5XYA9RXAgA=\n	ZeOmBDcYx8qJu7t3g+wRKiTopIxZTSqu\n	2011-12-14 11:58:00.218954
ZeOmBDcYx8qJu7t3g+wRKmWf5HjspnlAcIJVAwhz7o1QK7LXDan+mQ==\n	ZeOmBDcYx8qJu7t3g+wRKitlFwhuryHjNF0G4y9ksSj5mJYVummjP3UIoIQeDjTTzKTYI716\nCKk=\n	2011-12-14 11:58:00.244364
ZeOmBDcYx8qJu7t3g+wRKrQ1kblCsTH1\n	ZeOmBDcYx8qJu7t3g+wRKsBk6BzjVPAS\n	2011-12-14 11:58:00.283023
I6CbeBgRRvS0LBxnxDWGnAL5WNgwZq9qELtX72MIctY=\n	ZeOmBDcYx8qJu7t3g+wRKqdwBuopBprz\n	2011-12-14 11:58:00.308429
I6CbeBgRRvS0LBxnxDWGnO8c+ha/Dwn1yoaxasoF0p2UdAnB10W6RrWqvL8wa+T0\n	I6CbeBgRRvS0LBxnxDWGnKiNNfMn0chG\n	2011-12-14 11:58:00.334782
I6CbeBgRRvS0LBxnxDWGnMhiKuZF43WGDq36E5oY5uewzI3KYfq0Ow==\n	I6CbeBgRRvS0LBxnxDWGnHpYbsO+FBx58uaV6uHDUuT4NH/OTOu+oaQ7KplW0+QSg9icEPSq\nJIjiMrABZhf4eNhfaS6oVTcwZzD5FCCxfD6hprTc4Sfdq1vqFiij480r6MgQJQD6cxzXtFyz\nzZdMWWQbvsNXBw385bikjp+8KkoRVbSBV09Rg7pXnkbpelQU\n	2011-12-14 11:58:00.362447
I6CbeBgRRvS0LBxnxDWGnNYFi4nyh28QhPymCp8zNhKf0DivEUgULg==\n	I6CbeBgRRvS0LBxnxDWGnA7jTe2Anr0J\n	2011-12-14 11:58:00.38926
\.


--
-- Data for Name: distance; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY distance (track_a_id, track_b_id, featurevectortype_id, distancetype_id, value, inserted, updated) FROM stdin;
\.

--
-- Data for Name: featurevectortype; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY featurevectortype (id, name, version, dimension_x, dimension_y, parameters, class_id, inserted, updated) FROM stdin;
1	/7o8nbv9D7/IM1lngIh24OQa0LjWXQUE\n	1	60	1	/7o8nbv9D7/IM1lngIh24FDyo6dM6UaqgI9zqm4FXrUtWjEgRIfKn/GJmwWbfu4CTU8dYeUo\nV5M1HMTv8y3RqSmyy556QAdn0I/PGjXvUAUMezrYkV0kxvKzsr/MBKYoLlELMrIKMxBFkhe2\nnkgLQ8w5yW68vsDRsEUFBALryL68Hh+piHWogDzQ88XHYXiF8PEPd3Dazj58T662ezZK7KCC\n05G6IFY0bwCYmKgXmPMelv2Ei00Eqmt1xxL6umGT+NcQwhQwwjQJcvX7BfPkj/Z5pLy4jNoA\nA8gL5YjlCpWntxKXuh88uGGFj61K9ljIi3D+JHygoGU=\n	NUMERIC	\N	\N
\.



--
-- Data for Name: featurevector; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY featurevector (track_id, featurevectortype_id, data, file_id, inserted, updated) FROM stdin;
1	1	my97LlUnAH92tCLc0Ib4GkfFtTBuzQWuNpGtoxzMoI7EyDOxBBFDhqhpBUsPF0yOo7PYE8Jl\\012NMNGA4VkQaKHe6eju25hXcorcGT66z0/DKHxwqAfnPcMsfkmXG5hWo/AxakCOig7r5TubtsW\\012vVT+DE6sBYYIAwSNiKP/3k17qBQbsnLvCdTqKHP1OqmjnWRfga1JhCyiHFuHIUjmr4+c2sJH\\012st6//kV3J/2BU9sd1h/gwTDnJY1UVBm9OLbRAAoDJoku53BEoPgLe3gI8FYH7Lt9+7cTT1L5\\012ZhJvxkSWLT/M7N+w4AWv+djbxKy/NqQKl0gftAPqqKPeRfQs4BzqlVtWaVpruorfJeOs9GUo\\0123CqMGpzE+Lgp/b7AYEo0FhWQbNjIWdxnnl5IE9opO7AY+ZRLts1N4758MK0iuHzmvu0KsMQb\\012ziHlof+tEK2AdjqfS9dPALNEaV3d5bI5C5VSgnXbNviI2b/W5qZd+3eYWt4MJyCUpxe7wedF\\012tgG40vWGcG/G5i/f6c/vk2cf6bsKndP+F2XLbipndJidPzfaMT0uPnh+4Hi6yl65378FYsBx\\012HCwOr97gCEY3P1UWakkxoMwduTq1UXxSALyE44rhZDkBETIn25YNWDQrP85gla3zR2i0rTZ8\\012UW/Lr6L6zMnk/JGzr2Qdj1XAUP3RR5RDoioB0jwMmOYArd9APB89LK0t/W+ZCXHDA0a4MlMk\\012RnDSawRPaX8Goe++cX07TEB9GwCMIEtyvNZix7pdYlIxFAkG8wYu0CMhWZnowakOQ3VRugJW\\012BBcRSHkHnAAHn6aXltBnNX8BjgOc3EL20dWVVIIQA6wMRyYYBi4owjE0fX50IuejkLwSYp9j\\012kGlDRoap1XkMRB34W/po53kDdVLgLbp8dKLOTHj6fGgVbxDZ9cqXfu0rOvZA1zo/+fnRjr/5\\012Gm3GRIZTxHapZ7z/TNRLtYjKknZwU5y5DSHWBk25e232Q1kGhyWieZ5RO7Rn5zxCYxF6WOvN\\0125oSF3oo7HPMu4gyg7PCT5d0R2u2VsnCjc+6r+mtfFrS58T1i3TXpeO0vWdORf42qCPpJnmZH\\012317TQBrx//xtK7ICbPh2kfR9H6Gp2mibmMyD1HSAX2Qk8ycv0tloCxdJwrfzcZW0zo4ipRy+\\012ehykpKRGt8pj7Y7159aq7xFmArGm0QxmrT3FXTMZWU/gq1CVcDd8gs8QhkxkyjKdYAKNPd98\\012iJb6M6dpEbCIPwDL4OqO4NMmsNhyJRkotNYI66Bh2K+BVF8xdYGG1ccRutQJk0MGp6+OMQCo\\012Lgl3OZrhCun56atZzZcxAR6tKge5Jma2Dkt0/zeIWdu5033pJYCmXJAWfsw/hJKqM1HXk9Kp\\0125QwNGWy36J4+Dp8/aXpqA0tfr6q3Erpl8eIzcHiLCOEEo1fy5ZyoZKUOcoCVOYwo0XtVH/Wa\\012yrLIkQ5I9Sy72kTfv8s9e7zOBB4d1bglVIABEF1iVGzqx+tk57ju7w/8G+cMlVFo43YPCpWc\\0127Y4K6oHg7fuj18lov2soekYv5nKnGT2ziVOJIf2m1Ttz9TjLAE05trZJfGf/jROLAmJ29qIR\\012p/OrvzwCiE9zXhymoCFo/NEE/4kTrRz4q4VTpBTwpZsW040+8tiBa4Uq1ck4qjN7Cv8bcj1G\\012DLBpmNcqOfk9aGFm2qY=\\012	1	\N	\N
2	1	YYTOXLXvM6IuEZSXeG1+R+6cHvwF3XLj/nY1Ktsk6ZST6I4N80Tbc+kOl0a05Wq4kl/rH0ek\\012e0C/gjABfLv24ufzACkvGqeLpYlT06N3XfuLxbkJkTqYtlHNgJzjn/jRlN8EdwTOUNSrMH4a\\012DkLy79RJif7k+UfPOSEPtQjTSpwcWB64ubO415Nu42tITV9bHoG2XhfoPgy/Kf5b7kTdvBk3\\012CnmIOxQDOvYrHVU4UsBvEg6IU82QW6Ik02FJyJ4PPFHnaqnzlEWktSzTAON9gLm/3+QVPY41\\012JGU3AhcHHAld3NZBIDIAKn0X8zADXkAp67YYimCT9W8aqmcWNK5/pXeoiAeAmEI7VjFksMCz\\012lE2OHf6ScWTaIcl+2rw+b2SzDZCrhKje4LwQDlk7uY+EuhmMd40uazR1LZp4QnRYshBSAMat\\012HJLWYuVElHb/8tfmOyoDc/tXXRBH8/0Kb8Vw3KhNjPaY50ILy75+gTIbR5YB3/QnDmqVw4EH\\012PM9TxSHU+ptksXkPArjf2HQVBIql3vUW7Z5wf5r9tEyCqo9ECylgKzEXigRirlpQ6E42l4lN\\012DjJTE2fXouG1Tvp4YTiXB9ffsORoWtnUTy3VD/MsXTqxuO1H6NjLjBngwJ2wCjpeFbMPrmZg\\012bnZqRtKJZn08o0eqgtvFutLJq4/WJx3nAKVYG/3kI+GmSt6wWuY5QjqKF3VEoUeNkIXNnJJ7\\012ka995Hxy2bjJ8IzrwHnxoWtz2Iq3CSc9bPz3rFL2rZFRBldBw4TQ0s+YE0nT5/xJQXzLURdP\\0121ikxF456aQ9VaBtCF4b9k5AVYJn/bFyT0IOEe7c+J6+lv6slXh6CSy6PHHJBkDjy7b9Sfivx\\012xTLrmkq3OjIlVJyx3v8aAlq/nEV23wEdfthTlrf61C3HLw25uewtmDgZPr9hngupRAZFqgdh\\012OnucyfKSgKH+gsdasXzvKeDX+Cy0PrR42YQcfDzeIfaNzbsg+vtLbP7d+ythIyScYUHX3lug\\012NvUQfTExZZNcLG2LgCO4p57dpUwsdjMv9JqO8tAp91JueeMt7vly/kZEq5SNUSE8GlbpQSCE\\012N8pilCA8mwodhEE4GuuwI0SMIxa8wU6AW9tawm20dqVgcVfnu6fm7RwQl4FlOgVbjWy3/gcZ\\012O6A2N+HHVqa1jrGumAhqcFG8XEe+7aMyc6mMDd4lwVFFqyRqZrv1XAvbCi9KJ0m/eCnjc+EJ\\012DqPz/4JW4wxFZF39b79fnHXXCpvZ2zAxYH5rkWHjiUXEUuqpgtZKpkZh0yEDSI/MhlTIbPJL\\012QhXz5NOgTPF0cXJFxNgKe/eyC+hO8Zoy4F2p73GKdDwmcHjJ3d+KCF15YIL3/BB+42oxfP8w\\012kGLkNBOzY5br2Blt8eC2Vz3304ONrrln5tOTeY6TsgSS7OUqdeClR5X7mrTZ8opxZYQN8cGg\\012onubvwHe6FttjhX58JGVuyltsjBU+Hod4qI7ULIjXASFOCZ8plpYa1sm6P+7z1Mlvtem2sgu\\012DPOoLhvaas2auT6w4LInOZMizKcviaoU9XRj2vo0ZbPiJPNKRRA4cBsjF8vfAZAsMq6aGIc/\\012gqmqDIzcRj8STFdx8Vksa6XTR7WRgRMcMFfNujPaJiPJbStiEDCsy/tEebYSWUj++903vtgX\\012HikYcVY0\\012	3	\N	\N
\.




--
-- Data for Name: smafejob_addfile; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY smafejob_addfile (id, priority, file_uri, created, started, finished1, started2, finished2, finished, status, collection_name, log, external_key, guid) FROM stdin;
1	0	/home/ewald/code/smafe-trunk/test/resources/testsmall.mp3	2011-12-14 12:04:21.861249	2011-12-14 12:05:40.054788	2011-12-14 12:05:40.209773	\N	\N	2011-12-14 12:05:40.209773	OK	\N	-------------------------------\nsmafewrapd, pid=11876\n-------------------------------\n2011-12-14 12:01:43 ii Starting addfile task: id=1, file_uri=/home/ewald/code/smafe-trunk/test/resources/testsmall.mp3, external_key=\n2011-12-14 12:01:43 ii /home/ewald/code/smafe-trunk/test/resources/testsmall.mp3\n2011-12-14 12:01:43 ii mp3 file format assumed as file ends in .mp3\n2011-12-14 12:01:43 ii mixing stereo wave to mono\n2011-12-14 12:01:44 ii Writing track record to DB\n2011-12-14 12:01:44 ii Writing file record to DB\n2011-12-14 12:01:44 ii Writing feature vector record(s) to DB\n2011-12-14 12:01:44 ii File moved: /home/ewald/code/smafe-trunk/test/resources/testsmall.mp3 to /tmp/testsmall.mp3\n	\N	/home/ewald/code/smafe-trunk/test/resources/testsmall.mp3
2	0	/home/ewald/code/smafe-trunk/test/resources/testthird.mp3	2011-12-14 12:04:21.893366	2011-12-14 12:05:41.291274	2011-12-14 12:05:41.395102	\N	\N	2011-12-14 12:05:41.395102	OK	\N	-------------------------------\nsmafewrapd, pid=11876\n-------------------------------\n2011-12-14 12:01:44 ii Starting addfile task: id=2, file_uri=/home/ewald/code/smafe-trunk/test/resources/testthird.mp3, external_key=\n2011-12-14 12:01:44 ii /home/ewald/code/smafe-trunk/test/resources/testthird.mp3\n2011-12-14 12:01:44 ii mp3 file format assumed as file ends in .mp3\n2011-12-14 12:01:44 ii mixing stereo wave to mono\n2011-12-14 12:01:45 ii Writing file record to DB\n2011-12-14 12:01:45 ii No new features extracted for this file.\n2011-12-14 12:01:45 ii File moved: /home/ewald/code/smafe-trunk/test/resources/testthird.mp3 to /tmp/testthird.mp3\n	\N	/home/ewald/code/smafe-trunk/test/resources/testthird.mp3
3	0	/home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3	2011-12-14 12:04:21.918943	2011-12-14 12:05:41.982399	2011-12-14 12:05:42.157847	\N	\N	2011-12-14 12:05:42.157847	OK	\N	-------------------------------\nsmafewrapd, pid=11876\n-------------------------------\n2011-12-14 12:01:45 ii Starting addfile task: id=3, file_uri=/home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3, external_key=\n2011-12-14 12:01:45 ii /home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3\n2011-12-14 12:01:45 ii mp3 file format assumed as file ends in .mp3\n2011-12-14 12:01:45 ii mixing stereo wave to mono\n2011-12-14 12:01:45 ii Writing track record to DB\n2011-12-14 12:01:45 ii Writing file record to DB\n2011-12-14 12:01:46 ii Writing feature vector record(s) to DB\n2011-12-14 12:01:46 ii File moved: /home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3 to /tmp/testsmall10percent faster.mp3\n	\N	/home/ewald/code/smafe-trunk/test/resources/testsmall10percent faster.mp3
\.


--
-- Data for Name: distancejob; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY distancejob (featurevectortype_id, track_id, distancetype_id, smafejob_addfile_id, status, priority, created, started, finished) FROM stdin;
1	1	1	1	\N	0	2011-12-14 12:05:40.209773	\N	\N
1	1	2	1	\N	0	2011-12-14 12:05:40.209773	\N	\N
1	1	3	1	\N	0	2011-12-14 12:05:40.209773	\N	\N
1	1	4	1	\N	0	2011-12-14 12:05:40.209773	\N	\N
1	1	5	1	\N	0	2011-12-14 12:05:40.209773	\N	\N
1	2	1	3	\N	0	2011-12-14 12:05:42.157847	\N	\N
1	2	2	3	\N	0	2011-12-14 12:05:42.157847	\N	\N
1	2	3	3	\N	0	2011-12-14 12:05:42.157847	\N	\N
1	2	4	3	\N	0	2011-12-14 12:05:42.157847	\N	\N
1	2	5	3	\N	0	2011-12-14 12:05:42.157847	\N	\N
\.


--
-- Data for Name: distancetype; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

--COPY distancetype (id, inserted, updated, name) FROM stdin;
--1	\N	\N	9879d6db96fd29l2134fc802214163b95a
--2	\N	\N	0bb19742b194ff2e0f15b6b51l13cd5c19
--3	\N	\N	d9558li19989c7616d3d92d4ff1c3c0072
--4	\N	\N	5e685cdb19gl8405d3c169bb3d35485349
--5	\N	\N	46ff06ae696616afdf3aco695efd3db84c
--\.



--
-- Data for Name: featurevectorsegment; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY featurevectorsegment (segmentnr, track_id, featurevectortype_id, data, file_id, startsample, length, inserted, updated) FROM stdin;
0	1	1	Z4DK6nFukrwqBs103QjAAKo914bTWCfiLvlWUSNra/Nm+27P9UI3iPaNLNq/pojeQshmkySF\\012hyxNx5xSKkcFXaVZoH6E18UJUVk6JqMipbbnb1KStMLqd/mmMv+5hct12wrnIzo6HbwmQ1C6\\012QJqF3fX0H9bmEc258gg1c7mYfKbj7hlpz1+cKV7EcOXLojzxIrC+hu75Kd5YhEM1LZaiiYyj\\012bKrGvwVWzoKEUaLWKEDmu8+CmMDK+Pgn3ATSXpzSJ69s+W3rSdaVEPQHLPhZMOSqKPm8bHMd\\0126QV19kqTCWPwWj4Av4dMJiHXRowstSnFLRRJ7QTRDZiDufs22cvgNbkGES714F1679EDxHDm\\012lBFUzp++URj/iVN2GEFF+wVTllVSnTrQTXZg/DDM5jbihLUujHMCiEJ9zupu/eIfPKegQyh8\\012dCbkAL1K7Rc6GXKQYVHniU8X0VGDjVb5Qz+MoRHVj18ix6bY8WIs1LEc7p474JwSBQjX92Cu\\0129hf3hiBXejZthWF6s1v2X2kGwmE4TiYLX3gwexuvsA2D3La3o9jaHtIdv03UA+UOVM4h7bZI\\012xGGvFZN9eq+HHwDGbp65J12tBMSAOGsVQ/AMOAP5dO4UnUOfEBQPWo3UtTPZNi9P46sxfr7I\\012p5soq7tzzdMcK14PoRJ+VnC2Hjd6CvfR22UF8FUjnpAxf3D2EV13TWLsQsN6Bn61ErJIhYos\\012rYwLdxvoc75gM2H6Deg3J/S2sy6ldEihXRlUmi02RtSZZDfIsBHdAtUtCIUj1N6Rw2kdoGAp\\012ohytk7djYKVjB6PKirTpcIqDhGGL4x3wU4+60m5TIH1paLWPmq49AyEfQS/QjkCGwzNq9MRQ\\012Kq1NdbSx+PIwPP/MJ2F3ab795B1FRE+6GWVnWjtvCHBbF20maFP++9BIJJM488BiEUfuSJc8\\012EThdpLpK3djXttWx6FCT7SW6Jfc9K/B/yyt9KgNcsvqMSiHmlaWYerMqblJWQocX3qIO21l5\\012q7Qp/TpgkomcE5JtNGvk2LKWf2rthFSnwEMVq91rrNfz7mQdEwZ4akuRC+GY7xUmY+6wg6Y+\\012MNTNIxoWVQwk5IXuBW/7czzPkaWNqHhswyHENrIfsCVVRQPdhePXLHLp1Cyh/0AA7WD3YS9L\\012HK9OvdJ1vAedLq/RDK6YbhzL/p/ojMSfxSqx8c+K3jDbfoQO1FhPSd3GJgmLaU1InVOKUkSl\\012XLhkohch8+Oo7Do/IQIJcE7ZdTJ1Pjm86jYFNIUuPUOjNeKchcSBU7qvN7gTf9zlMtRdLrJs\\012nv88sV4snHurNwX288xTir2hMNmE4SoYIxy6bwbCgMxBqTPTd2xBLeugR/AeiPhdNXVV5Fa9\\012BvMnnp57yGQb6Jidjkpdacw+3iylWVSR5WFbVkjQAQDD61RR0Ksw2jEMYYcYAKoVjhLC45so\\012YkIyf0S/+thyq4O40zSRQLU3bwbeqBIGYeXZtKmYSUq6K6Qxtsx7es/XHIyFl+ZQOiijlgq7\\0122e7O/tle+gS9qKYGQeBtwUGsnea5WomD6HRAuBEGTvJRlVUoRYuk2uvH+6squtNyKkoIS7J/\\012cD9cq5Nb1zzBr++Lz606cyOrw/AhNxuB5MuywYAxT2n4erJz/ayqS2GhaKfg6snOGr7hYQ2M\\012jJsW+R+m0GQLFyrqqx4=\\012	1	262144	262144	\N	\N
0	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
1	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
2	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
3	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
4	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
5	2	1	yuXBhNrA+siWn8LUJKaZ7VRLvyw8aq5n+px0MvAEelXUrCR2JBVmQrXmSPD+BQ3f9BaQsxlB\\012wJP0YkRSZE8aaqgQfVcEU7n195ZgTX0TMUjEIuXNS6TYsbY2SJjl7FSvFVGWAtV1gsKGHxfc\\012DWStJ4pjXhsNPgoY/ldQaiGKoZ8jbS+XcL7Ngtog2veCCK97bgmmCK0fxFMCrQV1uiGjb8z1\\012nL0uJhI+GqLHHhD4s4fP1JBTjE2HnjcYNvJf833PmfDWduVaB/MWMVQ6juCVuq10Hia4OCQJ\\0124GxYwn1K4nHH/vM9XIbjXePHjyjTcFccsR/CYb+gQ0zQaLbce20+2+AVB6I/OF9WAKODkeNc\\012NWe5qEdoZw18qZWP+om5SqxfKP41w4FFCPsiALKNY3c/si7Kb1OyXOIrspQ7Ivirbd6IFZJE\\012m5M2wgHZNafhKlHq78ycLwsrlZ+QQkHjLTVyszlflZtgui27V2WyTpdsehwrqIl5tZuq/tSl\\012e5j/CZ8zbnKNdi4yOatXd7t8yiSvamufj4tUJ+e+o839LURjyTTWmIuHrYrE8jquE6OTyCja\\012Gix7tvxRZIdCO1DYYEQihNX/jdL8M0y+NERrGyblYpMn/TKL0+1IABWNl+P37mF+1Ac7XpDN\\012OOlaUbetccHiTw/8YxL57rpTcq1BkefQUBZATwFzEf0Dw76+0C/D71fQa2zjv6CBgYh0alQ0\\0126fHBhqg5KorVcc7JMUru+hTE//aB139YEXLkkOfhKLmM9i3zaDE3iGpsxdkNwyKox1t5l0dE\\012zMw/cZnOBiGraPHQR6pwXvZy/zR0+pzpjp1RqzIwHyqjFToKQWAS6Morzt2l4Tyclz0rXmGy\\01235pIPpe0wufJnfrRUEBonHmUdP7oba246bgog2Ft6nPXApGLKewCi4qX2I9VJAe3C3vEarft\\012fmNejOqa5D26yU9TRHb8IgUnEUOay5ho4uK8djDkOOnj1GMsoQBjgrFp4dZhMgW4uTMqzewh\\012qD69+dvzDXFKUkpGmbvx35iahWGBM0AOYfbCXyFsJAFf/xJeTdKvIoUpAr6R2taSM8JXOXZS\\012DApQmP73VB1MoBOBYllQiyasDL05TnjkWaJye6KiNPd2iqmfzFnTnY4EdJhjjQvGfbGj5gtD\\0121MXZylKvl9tZrxJNF+Q+b7fSnI2PzjpUNcW20Wso4XDhUp84H/L/ZJ9iBWTuAkcJlUuc5yn7\\012jR7QYm2YyGsJvuMESrOXBFkC5yaZU8VcCMn2Z8JcW8uI8OpnhKlkJVSluSbhbGDtiWBw2GFE\\012Y2jLNy8IWdeBzuTmBJ8Ca+m0cZe0vb/bKtlv4Zz9cXpHnVHNMbEU42NEI52bCkX36L059mri\\012MoVFep05lUAstzulouX1B8XhF6DIcTMLdH0dwCkbveAVoO4lBdR6WLyfOGFxgClZ/qJPVdYD\\012V0T97WP7guW3Vo2Tx2BYOl1A90AeMyfZWeqhoGvz4EEdhMxXL6yF+xmTER8JrUIhZZCgne5w\\012hu1Ysz9V+wee/0HQaSiZgolg46tYY3X4zjETJTiUBILSeE4+1IiGcUeTI5b9Jl1/t6sHfm9q\\012zjGMRqTWxUuKSW611I/d4bwDO0BkeGBADvCLXeMhmCUq0h9hR8ePsKydwyAC55ANFICFDi+L\\012BXTsIVt8\\012	3	262144	262144	\N	\N
\.





--
-- Data for Name: smafejob_deletecollection; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY smafejob_deletecollection (id, priority, collection_name, created, started, finished1, started2, finished2, finished, status, log) FROM stdin;
\.


--
-- Data for Name: smafejob_deletefile; Type: TABLE DATA; Schema: public; Owner: smafeadmins
--

COPY smafejob_deletefile (id, priority, collection_name, file_id, created, started, finished1, started2, finished2, finished, status, log) FROM stdin;
\.



