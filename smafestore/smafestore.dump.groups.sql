--
-- PostgreSQL database cluster dump
--

\connect postgres

SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET escape_string_warning = 'off';

--
-- Roles
--

CREATE ROLE smafeadmins;
ALTER ROLE smafeadmins WITH NOINHERIT CREATEROLE CREATEDB NOLOGIN;
CREATE ROLE smafeusers;
ALTER ROLE smafeusers WITH NOINHERIT NOCREATEROLE NOCREATEDB NOLOGIN;




--
-- PostgreSQL database cluster dump complete
--

