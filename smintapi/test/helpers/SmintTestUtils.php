<?php 
class SmintTestUtils {

  public static function validateXMLResponse($headers, $response, $expected_http_code, $mandatoryElements, $mandatoryAttributes, $testObject){
    
    $xml = null; 
    
    // check if http code fits 
    if ( $headers["http_code"] != $expected_http_code )  
      $testObject->fail( "Response has the wrong http code : " . $headers["http_code"] ."\n\n" . print_r($headers,true) . "\n\n" . $response );
    
    // parse xml result 
    try {
      $xml = simplexml_load_string($response);
    } catch (Exception $e) {
       echo "An error has been raised while processing the response: $response";
       $testObject->fail( "An exception has been raised: " . $e->getMessage() ."\n\n" . $e->getTraceAsString() );
    }
    
  
    if (!is_object($xml)) {
      $testObject->fail( "There was an error while processing the response: $response" );
    }

    $existingAttributes = array();
    $existingElements = array(); 
    self::getAllElementsAndAttributes($xml, $existingElements, $existingAttributes);
    

    foreach ($mandatoryElements as $key => $value) {
      // test if all attributes are included 
      $testObject->assertArrayHasKey( $value, $existingElements, "Element missing in result: \n\n" . $response);
    }
  
    foreach ($mandatoryAttributes as $key => $value) {
      // test if all attributes are included 
      $testObject->assertArrayHasKey( $value, $existingAttributes, "Attribute missing in result: \n\n" . $response);
    }

    return $xml; 

    }
    
    
    private static function getAllElementsAndAttributes($xmlNode, &$existingElements, &$existingAttributes){
      foreach ($xmlNode->children() as $elementName => $child) {
        $existingElements[$elementName] = $child; 
        foreach ($child->attributes() as $key => $value) {
          $existingAttributes[$key] = $value;
        
        }
        self::getAllElementsAndAttributes($child, $existingElements, $existingAttributes);
    }
  }
  
  
}