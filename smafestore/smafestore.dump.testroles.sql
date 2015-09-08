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

CREATE ROLE testsmafeadmin LOGIN;
ALTER ROLE testsmafeadmin WITH INHERIT   PASSWORD 'md56f5905f165ec7faaadfac2a49bd52da4';
CREATE ROLE testsmurf LOGIN;
ALTER ROLE testsmurf WITH INHERIT  PASSWORD 'md548ccd3f4abfb39713b554256c4899a18';


--
-- Role memberships
--
-- assume that groups have been created

GRANT smafeadmins TO testsmafeadmin;
GRANT smafeusers TO testsmurf;
--Uncomment this if you want testsmurf to be an admin.
--GRANT smafeadmins TO testsmurf;


--
-- PostgreSQL database cluster dump complete
--

