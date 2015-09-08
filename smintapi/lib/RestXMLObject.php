<?php 

class RestXMLObject extends RestObject {

  public function getResponseObject($contentType="application/xml"){

    switch ($contentType) {
      case 'application/xml':
          $content = $this->getXML();
          $this->response->setContent($content);
          return $this->response;
        break;

      default:
        $this->response->setStatus(406); 
        $this->response->setContent( SmintErrorXML::error($this->response->getStatusCode(), "The client does not accept any of the accepted content types. Try using a client that supports application/xml or add the header manually." ) );
        return $this->response;
        break;
    }
  }
  
  protected function getXML(){
    throw new Exception('RestXMLObject is abstract and should not be instantiated! Overwrite getXML method!');
  }
  
}




