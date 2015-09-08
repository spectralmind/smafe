<?php 

/**
 * MyLog provides a logger that will place the log file in the folder defined in the constant LOGFILEPATH
 * MyLog is singleton so no constructor, but the singleton method is used for instantiation 
 *
 * @package default
 * @author jochum
 **/
Class MyLog {
  
  public static $ALL    = -2147483648;
  public static $DEBUG  = 10000;
  public static $INFO   = 20000;
  public static $WARN   = 30000;
  public static $ERROR  = 40000;
  public static $FATAL  = 50000;
  public static $OFF    = 2147483647;
  
  protected $starttime; 
  protected $logLevel;
  
  protected $filepointer; 
  
  const LOGFILEPATH = '/../log/'; 
  
  // Hold an instance of the class
  private static $instance;
  
  /**
   * used to get the correct log level 
   *
   * @param string $levelAsString
   * @return log level, if wrong log level was provided ERROR log level will be returned
   * @author jochum
   */
  public static function getLevel($levelAsString){
    switch ($levelAsString) {
      case 'ALL': return self::$ALL;
      case 'DEBUG': return self::$DEBUG;
      case 'INFO': return self::$INFO;
      case 'WARN': return self::$WARN;
      case 'ERROR': return self::$ERROR;
      case 'FATAL': return self::$FATAL;
      case 'OFF': return self::$OFF;
      default: return self::$ERROR; 
    }
  }
  
  /**
   * private constructor, use singleton instead 
   *
   * @param string $logLevel 
   * @param string $logfilename 
   * @author jochum
   */
  private function __construct($logLevel, $logfilename) {
    $this->starttime = microtime(true);
    $this->logLevel = ( is_numeric($logLevel) ) ? $logLevel : self::getLevel($logLevel) ;  
    
    $filename = dirname(__FILE__). self::LOGFILEPATH . $logfilename;
    $filepointer = fopen($filename, 'a+');
    if ($filepointer === false) {
      throw new Exception("log file could not be opened for writing: $filename"); 
    }
    else $this->filepointer = $filepointer; 
  }
  
  
  public function __destruct() {
  }
  
  /**
   * The singleton method
   *
   * @param string $logLevel the level the log function should work with, all levels higher or equal will be logged 
   * @param string $logfilename optional filename to be used by the log method
   * @return instance of the MyLog class 
   * @author jochum
   */
  public static function singleton($logLevel=30000, $logfilename="api.log") 
  {
      if (!isset(self::$instance)) {
          $c = __CLASS__;
          self::$instance = new $c($logLevel,$logfilename);
      }
      return self::$instance;
  }
  
  
  /**
   * prints message and time until last message
   *
   * @param string $debugMessage the Message to be logged
   * @param string $messageLogLevel the Log level of the message, the message will only be logged if the Logger was instantiated with a higher or equal log level 
   * @return void
   * @author jochum
   */
  public static function printWithDuration( $debugMessage , $messageLogLevel = 10000)
  {
    $logger = MyLog::singleton();
    if ($messageLogLevel >= $logger->logLevel) {
      $fullMessage = RestUtils::getRealIpAddr() . " - " . date("[d/M/Y:G:i:s]"). " " . $debugMessage . " - took (ms): " . (microtime(true) - $logger->starttime)*1000 . "\n";
      $logger->writeToFile( $fullMessage );
      $logger->starttime = microtime(true);
    }
  }
  

  /**
   * performs the actual writing to the file
   *
   * @param string $fullMessage the message that is written to the file 
   * @return void
   * @author jochum
   **/
  private function writeToFile ( $fullMessage ) 
  {
    try {
      $fp = $this->filepointer;
    	$count = 0;
    	$loop = 10; 
    	$filewritten = false; 

      while( (!$filewritten) && ($count < $loop) ) {
         if (flock($fp, LOCK_EX)) {
            fwrite($fp, $fullMessage);
            $filewritten = true; 
            flock($fp, LOCK_UN);
         }
         $count++;
      }
    } catch (Exception $e) {
      // unlock if error pops up 
      flock($fp, LOCK_UN);
    }
    
    // file_put_contents locks the file and concurrent writes will fail !!! 
    // file_put_contents( $filename, $fullMessage, FILE_APPEND );
  }
    
}
