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

CREATE ROLE smafeadmin LOGIN;
ALTER ROLE smafeadmin WITH INHERIT  PASSWORD 'md57e66814bc192e045af832b677b906943';
CREATE ROLE smurf LOGIN;
ALTER ROLE smurf WITH INHERIT  PASSWORD 'md51eff0c4e7b7f2f88f2577dd4d047962f';


--
-- Role memberships
--

GRANT smafeadmins TO smafeadmin;
GRANT smafeusers TO smurf;


--
-- PostgreSQL database cluster dump complete
--

