<?php
class TestConfig
{
    // Hold an instance of the class
    private static $instance;
    private static $config;
    
    // A private constructor; prevents direct creation of object
    private function __construct() 
    {
      // load the config file
      $path = dirname(__FILE__)."/../test.ini";
      self::$config = parse_ini_file($path, true);
    }

    // The singleton method
    private static function singleton() 
    {
        if (!isset(self::$instance)) {
            $c = __CLASS__;
            self::$instance = new $c;
        }

        return self::$instance;
    }
    
    public static function getConfig()
    {
        $aSingletonInstance = self::singleton();
        return self::$config;
    }

    // Prevent users to clone the instance
    public function __clone()
    {
        trigger_error('Clone is not allowed.', E_USER_ERROR);
    }
    
    
    public static function getLocalTestFiles() {
      $testfileslocal = array(
        array("F1.mp3","",""),
        array("F1.mp3","","A"),
        array("F1.mp3","","B"),
        array("F1.mp3","","C"),

        array("F1.mp3","mykey1-d",""),
        array("F1.mp3","mykey1-A","A"),
        array("F1.mp3","mykey1-B","B"),
        array("F1.mp3","mykey1-C","C"),

        array("F1.mp3","myotherkey1-d",""),
        array("F1.mp3","myohterkey1-A","A"),
        array("F1.mp3","myotherkey1-B","B"),
        array("F1.mp3","myotherkey1-C","C"),

        array("F2.mp3","mykey2-A","A"),
        array("F3.mp3","mykey3-A","A"),
        array("F4.mp3","mykey4-A","A"),
        array("F5.mp3","mykey5-A","A"),

        array("F2.mp3","mykey2-B","B"),
        array("F3.mp3","mykey3-B","B"),
        array("F4.mp3","mykey4-B","B"),
        array("F5.mp3","mykey5-B","B"),

        array("F1 copy.mp3","myotherotherkey1-d",""),
        array("F1 copy.mp3","myohterotherkey1-A","A"),
        array("F1 copy.mp3","myotherotherkey1-B","B"),
        array("F1 copy.mp3","myotherotherkey1-C","C"),

        array("F2 copy.mp3","myotherkey2-A","A"),
        array("F3 copy.mp3","myotherkey3-A","A"),
        array("F4 copy.mp3","myotherkey4-A","A"),
        array("F5 copy.mp3","myotherkey5-A","A"),

        array("F2 copy.mp3","myotherkey2-B","B"),
        array("F3 copy.mp3","myotherkey3-B","B"),
        array("F4 copy.mp3","myotherkey4-B","B"),
        array("F5 copy.mp3","myotherkey5-B","B"),
        );
      
      return $testfileslocal; 
    }
    
    public static function getRemoteTestFiles() {
      $config = TestConfig::getConfig();
      $file_urls = $config['testaddurl'];
      $testfilesremote = array();
      foreach ($file_urls as $ext_key => $url) {
        $testfilesremote[] = array ($url, $ext_key, "");
        $testfilesremote[] = array ($url, $ext_key . "-C", "C");
      }
      return $testfilesremote; 
    }    
    

}
