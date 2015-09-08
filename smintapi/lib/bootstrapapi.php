<?php 

// I expect $CONFIGFILENAME to be set, otherwise bail out
if (isset($CONFIGFILENAME))  {
	$apiConfig = SmintapiConfig::initConfig( $CONFIGFILENAME);
	$apiConfig = SmintapiConfig::getConfig( );
} else
	throw new Exception('Please set $CONFIGFILENAME to the config file to be used.');

// init logger with log level defined in the config file 
  $mylog = MyLog::singleton($apiConfig["logging"]["logLevel"]);


// define the autoloader to load classes from the lib folder
function __autoload($className){
  $filename = dirname(__FILE__).'/../lib/'.$className.'.php'; 
  
  if (file_exists($filename)) require $filename;
  else throw new Exception('Class "' . $className . '" could not be autoloaded. File not found: '.$filename);
}

function stripslashes_deep($value)
{
    $value = is_array($value) ?
                array_map('stripslashes_deep', $value) :
                stripslashes($value);

    return $value;
}