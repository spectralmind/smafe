<?php 

// init logger 
  $mylog = MyLog::singleton(MyLog::getLevel("ALL"), "test.log");
  

// define the autoloader to load classes from the lib folder
function __autoload($className){
  if ( in_array( $className, array( "TestConfig", "SmintTestUtils" ) ) ) {
    $filename = dirname(__FILE__).'/'.$className.'.php'; 
  } else {
    $filename = dirname(__FILE__).'/../../lib/'.$className.'.php'; 
  }
  
  
  if (file_exists($filename)) require $filename;
  else throw new Exception('Class "' . $className . '" could not be autoloaded. File not found: '.$filename);
}

 