<?php
// $Id$
/* =============================================================================
 * idu.inc - Functions and an extended class to support
 * Islandora Drush Utilities and dependent modules.
 * 
 * Authors:  Mark A. McFate
 *
 */

/*
 * Class MyXML extends SimpleXMLElement
 */
class MyXML extends SimpleXMLElement {
  
  /* -------------------------------------------------------------------------
   * find - Accepts an Xpath statement and harvests all namespaces from
   * the document then registers each prior to evaluating the xpath.
   * Important!  If your XML contains a default (blank) namespace prefix, 
   * you MUST use a prefix of 'x:' in your xpath!
   * 
   */

    public function find( $xpath ) {

      // Get all namespaces from the XML and register their prefixes so that xpath works!
      
      $namespaces = $this->getNamespaces( TRUE );
      foreach ( $namespaces as $prefix => $ns ) {
        if ( strlen( $prefix ) == 0 ) $prefix = 'x';     // use 'x:' in place of a blank!
        $this->registerXPathNamespace( $prefix, $ns );
      }
        
      $tmp = $this->xpath( $xpath );
      return isset( $tmp[0] ) ? $tmp : null;
    }

    public function remove( ) {
        $dom = dom_import_simplexml( $this );
        return $dom->parentNode->removeChild( $dom );
    }

    
    public function appendXML( $append ) {
      if ( $append ) {
        if ( strlen( trim(( string ) $append )) == 0 ) {
          $xml = $this->addChild( $append->getName( ));
        } else {
          $xml = $this->addChild( $append->getName( ), ( string ) $append );
        }

        foreach ( $append->children( ) as $child ) {
          $xml->appendXML( $child );
        }

        foreach ( $append->attributes( ) as $n => $v ) {
          $xml->addAttribute( $n, $v );
        }
      }
    }

}

/* Example: removing the <bar> element with id = 1
$foo = new MyXML( '<foo><bar id="1"/><bar id="2"/></foo> ');
$foo->find( '//bar[@id="1"]' )->remove( );
print $foo->asXML( ); // <foo><bar id="2"/></foo>
*/


/* -----------------------------------------------------------------------------
 * pidn( $pid )
 * 
 * Utility function to return the numeric portion of a PID as an integer.
 *   
 */
function pidn( $pid ) {
  list( $ns, $pidn ) = explode( ':', $pid );
  return intval( $pidn );
}

/* -----------------------------------------------------------------------------
 * curl_query( $url )
 * 
 * Utility function to perform a CURL query and return the result.
 *   
 */
function curl_query( $url ) {
  $username = $_SERVER['FEDORA_GSEARCH_USER'];
  $password = $_SERVER['FEDORA_GSEARCH_PASS'];

  $curl = curl_init( );
  curl_setopt( $curl, CURLOPT_URL, $url ); 
  curl_setopt( $curl, CURLOPT_TIMEOUT, 30 ); //timeout after 30 seconds
  curl_setopt( $curl, CURLOPT_HEADER, 0 );
  curl_setopt( $curl, CURLOPT_RETURNTRANSFER, true );  // to return the response rather than printing it
  curl_setopt( $curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY );
  curl_setopt( $curl, CURLOPT_USERPWD, "$username:$password" );

  $status_code = curl_getinfo( $curl, CURLINFO_HTTP_CODE );   //get status code

  if ( $status_code != 0 ) {
    drupal_set_message( "CURL returned a non-zero status code ($status_code) for '$url'.", 'error' );
    $content = FALSE;
  } else {
    $content = curl_exec( $curl );
  }    
  curl_close( $curl );
  return $content;
}

/**
 * Tokenize a string forSolr query *t field rather than 'full string' *s field.
 *
 * Break at all non-word characters and drop all stopwords, AND if $counter>0
 * drop the last token.  This way the function can e called iteratively to
 * gradually reduce the search string one token at a time.  The function returns
 * the new string of tokens.
 *
 * @param string $title - The string to be tokenized.
 * @param int $counter
 * @return string
 */
function tokenize_for_solr( $title, $counter=0 ) {
  $stopwords = 'a,an,and,are,as,at,be,but,by,for,if,in,into,is,it,no,not,of,on,or,s,such,t,that,the,their,then,there,these,they,this,to,was,will,with';
  $ignore = explode( ',', $stopwords );
  $keywords = preg_split( "/[^a-zA-Z_0-9]+/", $title, NULL, PREG_SPLIT_NO_EMPTY );
  $limit = count( $keywords );
  if ( $counter > 0 ) { $limit--; } 
  $query = "";
  
  for ($k=0; $k<$limit; $k++) {
    $lcToken = strtolower($keywords[$k]);
    if (!in_array($lcToken, $ignore)) { $query .= "$lcToken "; }
  }
  
  return trim($query);
}

/**
 * Returns num_found from a Solr query of $pid vs. dc.identifier.
 * 
 * @param string $pid
 *   A Fedora object PID.
 * @return boolean|integer
 *   FALSE if $pid is not found in dc.identifier or the number found.
 */
function icg_query_for_pid($pid) {
  $qp = new IslandoraSolrQueryProcessor();
  $query = 'dc.identifier:"'.$pid.'"';
  $qp->buildQuery($query);
  $qp->solrParams['fl'] = "PID";
  $qp->executeQuery();
  $response = $qp->islandoraSolrResult['response'];
  $numFound = $response['numFound'];
  if ($numFound > 0) {
    return $numFound;
  } else {
    return FALSE;
  }
}

/**
 * Queries the Solr index (assumes 'collection1' core) for $title in $q.
 * 
 * @param string $title
 *   The string to search for in the specified index/field, $q.
 * @param string $q
 *   The solr index/field to query, with no trailing colon!
 * @param SimpleXMLElement $result
 *   The entire XML $result is returned here.
 * @param string $x
 *   Optional 'extra' query parameters.  Must be fully specified in the form
 *     index:target as this will be appended to $q:$title with an AND operation.
 *
 * @return integer
 *   Returns the value of num_found or FALSE on failure.
 */
 function query_solr_index( $title, $q, &$result, $x=NULL ) {
  $repo = variable_get( 'icg_tuque_URL' );
  $solr = str_replace( '/fedora', '/solr', $repo );
  
  // Before the query, commit all pending Solr documents with http://solrhost:8080/solr/update?commit=true
  $url = $solr.'/update?commit=true';
  if ( !$result = curl_query( $url )) {
    drupal_set_message( 'Solr commit failed (returned FALSE) in query_solr_index!', 'error' );
    return FALSE;
  }
  
  // We need to urlencode( ) the title before the search.
  $t = urlencode( $title );
  
  // Also, the search may not contain any single or double quotes, so remove them 
  // before searching.
  if ( strstr( $t, "'" ) || strstr( $t, '"' )) {
    die( "Stopped in query_solr_index: The search string '$t' contains quotes!" );
  }

  // @TODO...implement argument $x

  // Attempt to query Solr's $q field for the target title.
  $url = $solr.'/collection1/select?q='.$q.':"'.$t.'"&fl=PID';
  $result = curl_query( $url );
  $content = trim( $result );
  
  // Fetch Num Found.
  $xml = new SimpleXMLElement( $content );       // open record as searchable XML
  $xpath = '/response/result/@numFound';
  $node = $xml->xpath( $xpath );
  $num_found = intval((string) $node[0]);
  
  // If $num_found is ONE, return the target PID only in $result rather than
  // returning the whole result document!
  if ( $num_found == 1 ) {
    $xpath = "/response/result/doc/str";
    $node = $xml->xpath( $xpath );
    $result = (string) $node[0];
  }
  
  return $num_found;
}

/* -----------------------------------------------------------------------------
 * idu_update_solr_index( $pid )
 *
 * Does just what it says, for one target PID.
 * 
 */
function idu_update_solr_index( $pid ) {
  
  // Attempt to update the FedoraGSearch index first.
  $fedora = variable_get('icu_Connect' );
  $url = $fedora.'gsearch/rest/rest?operation=updateIndex&action=fromPid&value='.urlencode($pid);
  $result = curl_query( $url );

  return $result;
}

/**
 * Save an ICG input form field as a Drupal variable with a prefix of 'icg_'.
 * 
 * @param string $name
 *   The name of the form variable in $formStateValues.
 * @param array $formStateValues
 *   The array, $form_state['values'] or equivalent, which holds $name.
 */
function icg_set_variable($name, $formStateValues) {
  $icgName = 'icg_' . $name;
  if (isset($formStateValues[$name])) {
    variable_set($icgName, $formStateValues[$name]);
  }
}

/**
 * Increments a variable, or set to 1 if it's not been set yet.
 * 
 * @param mixed $var
 */
function icg_increment($var) {
  if (isset($var)) {
    ++$var;
  } else {
    $var = 1;
  }
}

/**
 * Appends text to a string which may not already exist.
 * 
 * @param string $var
 * @param string $text
 */
function icg_append(&$var, $text) {
  if (isset($var)) {
    $var .= $text;
  } else {
    $var = $text;
  }
}

/**
 * Returns an array of MIME types for known, non-original content datastreams.
 * 
 * @staticvar array $ds
 * @return array
 */
function rcDSMimes( ) {
  static $ds = array( 'RELS-EXT' => 'application/rdf+xml',
                      'DC' => 'text/xml',
                      'MODS' => 'text/xml',
                      'RELS-INT' => 'application/rdf+xml',
                      'TN' => 'image/jpeg',
                      'POLICY' => 'text/xml',
                      'PREVIEW' => 'image/jpeg',
                      'JPG' => 'image/jpeg',
                      'JP2' => 'image/jp2',
                      'MEDIUM_SIZE' => 'image/jpeg',
                      'FULL_TEXT' => 'text/plain',
                      'OCR' => 'text/plain',
                      'RAW_HOCR' => 'text/xml',
                      'HOCR' => 'text/html',
                      'ENCODED_OCR' => 'text/xml',
                      'islandora_workflow' => 'text/xml',
                      'OriginalDC' => 'text/xml',
                      'PROXY_MP3' => 'audio/mpeg',
                      'MP4' => 'video/mp4',
                    );
  return $ds;
}

/**
 * Returns the DSIDs for known, not-original-content datastreams.
 * 
 * @return array
 */
function rcDSIDs( ) {
  return array_keys(rcDSMimes( ));
}


/**
 * Return the proper MIME type for an OBJ datastream based on content model.  Some have * because they can vary.
 * 
 * @return mixed
 */
function rcOBJMimes( ) {
  static $ds = array( 'sp_pdf' => 'application/pdf',
                      'sp_basic_image' => 'image/jpeg',
                      'sp_large_image_cmodel' => 'image/tiff',
                      'compoundCModel' => 'NONE',
                      'binaryObjectCModel' => 'application/*',
                      'bookCModel' => 'image/tiff',
                      'pageCModel' => 'image/tiff',
                      'sp-audioCModel' => 'audio/*',
                      'sp_videoCModel' => 'video/*',
                    );
  return $ds;
}

/**
 * Attempt to make derivatives for a specified FedoraObject.
 *
 * @param FedoraObject $object
 *   The object to be processed.
 *
function icg_make_derivatives($object) {

  // Get the object's content model.  Hopefully it has only one!
  $object_content_models = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
  $contentModel = $object_content_models[0][object][value];

  // Create new derivatives based on $contentModel
  switch ($contentModel) {
    case 'islandora:sp_basic_image' :
      islandora_basic_image_create_thumbnail($object);
      islandora_basic_image_create_medium_size($object);
      break;
    case 'islandora:sp_large_image_cmodel' :
      islandora_large_image_create_all_derivatives($object);
      break;
    case 'islandora:sp_pdf' :
      islandora_pdf_add_fulltext_derivative($object);
//    islandora_pdf_add_jpg_derivative( $object );        // not sure this is a good idea, seems to hang?
      islandora_pdf_add_preview_derivative($object);
      islandora_pdf_add_tn_derivative($object);
      break;
    case 'islandora:binaryObjectCModel' :
      islandora_binary_object_create_thumbnail($object);
      break;
    case 'islandora:sp-audioCModel' :
      // islandora_audio_create_mp3($object);    // not enabled?
      break;
    case 'islandora:pageCModel' :
    case 'islandora:bookCModel' :
    case 'islandora:compoundCModel' :
    case 'islandora:sp_videoCModel' :
    case 'islandora:sp_web_archive' :
      drupal_set_message("Derivatives cannot be created in icg_make_derivatives for '$contentModel' object $object->id.", 'warning');
      break;
  }
}

/**
 * Make a new child (of compound) or stand-alone Fedora object in the specified CModel and collection.
 * 
 * @param FedoraObject $object
 *   The parent or original object.  The new object becomes part of the same collection as $object.
 * @param FedoraDatastream $cds
 *   The content datastream to be ingested.
 * @param string $contentModel
 *   The content model for the new object.
 * @param int $seq
 *   An auto-incremented sequence number for the new child.
 * @param string $new_label
 *   The new label for the object, or NULL if none is provided.
 * @param boolean|string $hasParent
 *   If FALSE the function makes a new stand-alone object rather than a 
 *   child of a compound parent.  Otherwise $hasParent specifies the PID of the parent.
 * @return string
 */
function icg_make_new_object($object, $cds, $contentModel, $seq, $new_label, $hasParent=FALSE) {
  $repo = $object->repository;
  $mime = $cds->mimetype;
  $ns = substr(variable_get('icg_namespace'), 0, -1);  // no colon on the namespace here!
  $collection = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');

  // Create a whole new child object...
  if ($hasParent) {
    $child_object = $repo->constructObject($ns);   // construct a new object in the target namespace
    $child_object->models = array($contentModel);   // associate it with $contentModel
    $child_object->label = ( isset($new_label) ? $new_label : $cds->id );   // use the label provided or the old DSID as new label/title
    $child_object->owner = $object->owner;
    
  // Or if this is a new stand-alone (not a child) object just reuse the existing 
  // object.  Start by removing old model and collection info and replace with new.
  } else {
    $child_object = $object;
    $child_object->relationships->remove(FEDORA_MODEL_URI, 'hasModel', 'info:fedora/islandora:genericCModel', TRUE);
    $child_object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'info:fedora/islandora:genericCollection', TRUE);
    $child_object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $contentModel);
  }
  
  // Add necessary relationships to the new object.
  foreach ($collection as $col) {
    $child_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $col[object][value]);
  }
  
  // Copy the constituent datastream to become the managed 'OBJ' of the new object.
  if (strstr($mime, 'tif')) {           // set the new MIME type
    $mime = 'image/tiff';
  } else if (strstr($mime, 'image/j')) {
    $mime = 'image/jpeg';
  }
 
  $new_OBJ = icg_copy_datastream($object, $cds->id, $mime, 'M', 'OBJ', $cds->id);
  $child_object->ingestDatastream($new_OBJ);

  // Add the parent-child relationship.
  // If this is a new child, copy the old object's MODS and POLICY to the new object.

  if ($hasParent) {
    $child_object->relationships->add(FEDORA_RELS_EXT_URI, 'isConstituentOf', $hasParent);

    $escaped_pid = str_replace(':', '_', $hasParent);
    $child_object->relationships->add(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid", $seq, TRUE);

    $modsCopy = icg_copy_datastream($object, 'MODS', 'text/xml', 'X');
    $child_object->ingestDatastream($modsCopy);

    if ($object['POLICY']) {
      $policyCopy = icg_copy_datastream($object, 'POLICY', 'text/xml', 'X');
      $child_object->ingestDatastream($policyCopy);
    }

    // Ingest the new object into the repository.
    try {
      $repo->ingestObject($child_object);
    } catch (Exception $e) {
      die('Caught Fedora exception: '.$e->getMessage());
    }
  }

  // Try to make necessary derivatives and update Solr.
  icg_make_derivatives($child_object);

  return $child_object->id;
}

/**
 * Copy a datastream from one object to another.
 * 
 * @param FedoraObject $fromObject
 *   The FedoraObject to copy from.
 * @param string $fromDSID
 *   The datastream ID to be copied.
 * @param string $mime
 *   The MIME type of the datastream to be created.  If not specified the MIME type of the old datastream is preserved.
 * @param string $controlGroup
 *   The controlGroup (X, I, M, etc.) of the datastream to be created.  If not specified the controlGroup of the old datastream is preserved.
 * @param string $newID
 *   The new DSID to be created, if specified.  If not specified the DSID of the old datastream is preserved.
 * @param string $newLabel
 *   The new datastream label to be created, if specified.  If not specified the label of the old datastream is preserved.
 * @return FedoraDatastream
 *   The new FedoraDatastream object, not yet ingested.
 */
function icg_copy_datastream($fromObject, $fromDSID, $mime=NULL, $controlGroup=NULL, $newID=NULL, $newLabel=NULL) {
  $cg = ($controlGroup ? $controlGroup : $fromObject[$fromDSID]->controlGroup);
  $newDS = ($newID ? $fromObject->constructDatastream($newID, $cg) : $fromObject->constructDatastream($fromDSID, $cg));

  $newDS->label = ( $newLabel ? $newLabel : $fromObject[$fromDSID]->label );
  $newDS->mimetype = ( $mime ? $mime : $fromObject[$fromDSID]->mimetype );
  $temp_file = drupal_tempnam('temporary://', 'datastream');  // open a temp file
  $fromObject[$fromDSID]->getContent($temp_file);  // copy DS binary content to the temp file
  $newDS->setContentFromFile($temp_file);          // set new DS content from the temp file
  drupal_unlink($temp_file);               // discard the temp file
  return $newDS;
}

/**
 * Helper function to fetch the $repo repository object.
 * 
 * @return FedoraRepository
 */
function icg_getRepo( ) {  
  if (!$repo = repository_control_connect( )) {
    $caller = debug_backtrace( DEBUG_BACKTRACE_IGNORE_ARGS, 2)[1]['function'];
    die("Error in $caller...no connection to Fedora!");
  } else {
    return $repo;
  }
}

