<?php 
class SmintErrorXML extends SmintXML {

  public static function getErrorMessage($code){
    if ($code < 600) {
      // 0-599 are reserved and mapped dirctly to the 
      return ResponseObject::getStatusCodeMessage($code);
    }
    else {
        // all smint api errors should be grouped into differnt categories, categories should be identified by a range of numbers 
        // e.g. database related error codes are between 5000 and 5999 
      $errors = array(
        "5000" => "General Database Error.",
        );
    }
    
  }
  
  public static function simpleErrorAsXML($code) {
    $xml = new SimpleXMLElement(SmintapiConfig::getXMLrootElement());
    $message = self::getErrorMessage($code);
    $error = $xml->addChild("error", $message );
    $error->addAttribute("code", $code );
    return $xml;
  }
  
  public static function simpleError($code) {
    $xml = self::simpleErrorAsXML($code);
    return $xml->asXML();
  }
  
  public static function error($code, $errorMessage) {
    $xml = self::simpleErrorAsXML($code);
    $xml->addChild("errorDetail", $errorMessage);
    return $xml->asXML();
  }
  
  
}