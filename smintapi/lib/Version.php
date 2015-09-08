<?php 

class Version extends RestXMLObject {
  
  protected function getXML(){
    $major = 1;
    $minor = 3;
    //$svn_revision = explode(' ','');
    //$revision = $svn_revision[1];
    $revision = 999;   // currently NA
    
    $content = SmintXML::simpleXMLElement();
    $version = $content->addChild("version");
    $version->addAttribute("major",$major);
    $version->addAttribute("minor",$minor);
    $version->addAttribute("revision",$revision);
    
    $this->response->setStatus(200); 
    
    return $content->asXML();
  }
  
}