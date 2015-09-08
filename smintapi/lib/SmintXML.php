<?php 
class SmintXML {
  
  public static function simpleXMLElement() {
    return new SimpleXMLElement(SmintapiConfig::getXMLrootElement());
  }
  
}