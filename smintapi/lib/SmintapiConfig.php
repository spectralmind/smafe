<?php
class SmintapiConfig
{
    // Hold an instance of the class
    private static $instance;
    private static $apiConfig;
    private static $dbconnection;
    
    // A private constructor; prevents direct creation of object
    private function __construct($configfile) 
    {
      // load the config file
      self::$apiConfig = parse_ini_file($configfile, true);
      
      
      
      // connect to the database if details specified
      if (isset(self::$apiConfig["database"]["dbname"])) {
	      $dbname = self::$apiConfig["database"]["dbname"];
	      $dbhost = self::$apiConfig["database"]["dbhost"]; 
	      $dbport = self::$apiConfig["database"]["dbport"]; 
	      $dbuser = self::$apiConfig["database"]["dbuser"];
	      $dbpassword = self::$apiConfig["database"]["dbpassword"]; 
	      self::$dbconnection = new PDO("pgsql:dbname=$dbname;host=$dbhost;port=$dbport", $dbuser, $dbpassword, array(PDO::ATTR_PERSISTENT => true));
      }
    }

    // The singleton method
    private static function singleton_name($configfile) 
    {
        if (!isset(self::$instance)) {
            $c = __CLASS__;
            self::$instance = new $c($configfile);
        }

        return self::$instance;
    }
    // The singleton method without config file name
    private static function singleton() 
    {
        if (!isset(self::$instance)) {
            $c = __CLASS__;
            self::$instance = new $c("Please use initConfig(..) before this call.");
        }

        return self::$instance;
    }
    
    public static function initConfig($configfile)
    {
        $aSingletonInstance = self::singleton_name($configfile);
    }
    
    public static function getConfig()
    {
	$aSingletonInstance = self::singleton();
        return self::$apiConfig;
    }

    public static function getXMLrootElement()
    {
        $aSingletonInstance = self::singleton();
        return self::$apiConfig["xml"]["rootnode"];
    }
    
    public static function getDBConnection()
    {
        $aSingletonInstance = self::singleton();
        return self::$dbconnection;
    }
    
    

    // Prevent users to clone the instance
    public function __clone()
    {
        trigger_error('Clone is not allowed.', E_USER_ERROR);
    }

}
