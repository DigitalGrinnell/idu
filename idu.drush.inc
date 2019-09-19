<?php

/* Attention! Replace all calls to 'idu_repo()' with '_idu_Repository()' */

/**
 * Some available drush functions...
 *
 * drush_log('Log an event using drush', 'warning');
 * drush_set_error('Set an error with drush.');
 * t('Translate strings with drush');
 * drush_print('Print to command line with drush');
 * drush_print_table($rows, TRUE); //print a command line table with drush
 * drush_confirm('Are you sure you want to continue?', $indent = 0); //Add drush confirmation
 *
 */

/**
 * This function looks for $fix in $verified (below) and prints a message or dies.
 */
function iduF_validate($fix) {
  $verified = array(                // $verified holds the verification/availability status of iduF commands.
    'ChangeText' => '3-Apr-2019',
    'Derivatives' => '10-Mar-2019',
    'IndexSolr' => '10-Mar-2019',
    'MigrateObject' => '3-Apr-2019',
    'Purge' => '10-Mar-2019',
    'RemoveEmptyCues' => '18-Mar-2019',
  );

  if (!key_exists($fix, $verified)) {
    die("Sorry, iduF command '$fix' is not verified!  ");
  } else {
    drush_log("Ok, iduF command '$fix' was verified on $verified[$fix].", 'status');
  }
}


/**
 * Command definitions and processing utilities follow here, generally in alphabetical order.
 */

/**
 * Implements hook_drush_command().
 */
function idu_drush_command( ) {
  $items = array();

  $items['iduFix'] = array(
    'description' => t("Attempts to apply the specified 'fix' to one or more existing PIDs."),
    'arguments'   => array(
      'first' => t("First PID to process. The namespace (ns) portion of this PID defines the default primary namespace.  This parameter may be specified as a .csv file path (like public://objects.csv), or 'ns:first-last' in place of the --repeat option, or 'ns:*' to operate on all collection objects.  A special entry in the form of ns:last can be used to specify starting at the highest ns:PID number which has been indexed in Solr.\n"),
      'fix'   => "Completes the name of the drush_idu_fix_NAME function to be called for each processed object.  Available Fix functions are:\n

      --- The first 3 commands may be automatically executed
          during ingest of a new object ---

      CompoundTN -
        Creates a CompoundTN datastream for a compound parent object.
          The new image datastream is constructed from up to four
          child object thumbnails.

      SelfTransform -
        Applies the specified --self self-transform to the specified
          --dsid datastream.  Use --reorder to also reorder MODS.
          [Verified 27-Apr-2018]

      UpdateDC -
        Ensures that specified objects have NO empty or missing
          dc:date or dc:creator fields.  This function also populates a
          /mods/extension/creators field from which dg_creators_s is
          derived.  Default values for date and creator are
          'Unspecified' and 'Unknown'.  Use --fixName to ensure that
          MODS records have a valid /name/namePart[not(@*)] element.

          Options:
            --hidden=FALSE  If TRUE a /mods/extension/hidden_creators element is
               created instead of /mods/extension/creators.

      --- The commands which follow are never automatically executed ---

      AddDatastream -
        Adds the --dsid datastream with --title and --contents to an
          existing object.

        Option defaults are:
          dsid=WF,
          title='Islandora Webforms List',
          contents=' 1934660 '

      AddXML -
        Adds a single XML element named --title, with a value of
          --contents, to an existing object's --dsid datastream.  One
          element is appended to one existing parent element identified
          by --xpath.

        Option defaults are:
          dryRun=FALSE,
          dsid=MODS,
          xpath=/mods:mods/mods:originInfo,
          title=dateCreated, and
          contents=date('now')

      ChangeDatastream -
        Changes the --dsid datastream's LABEL to --title, OR its ID to
          --replace for an existing object.

      ChangeText -
        Changes --find text to --replace either in general or within the
          value of a specified --xpath.

        Options: find, replace, substitutionsFile, dsid, xpath, dryRun.

      ChildSync -
        Synchornize characteristics of child objects with their parent.
          This function only touches objects where the isConstituentOf
          relationship exists.

          Characteristics include:
            Child will inherit the same isMemberOfCollection values as
            the parent.

      Clean -
        Removes all unnecessary datastreams from an existing object.

      CModelToMODS -
        Copy object's /rdf:RDF/rdf:Description/...
          fedora-model:hasModel[@rdf:resource] attribute from the RELS-EXT
          datastream to its /mods/extension/CModel.

      ConvertTIFtoJP2 -
        Converts an object OBJ datastream to a JP2 using options
          specified by --convert or --compress.

      CopyDatastream -
        Copies the named --dsid datastream from a specified --parent object
          to the target object.  [Verified 2-May-2018]

      CorrectCollections -
        Corrects collection assignments including removal of duplicate
          collection relationships, and removal of --retire collection
          references IF other relationships exist.

      CreateIslandoraBags -
        Initiates one or more 'drush --user=1 create-islandora-bag object PID'
          commands to generate Bagit containers of object datastreams.

      DateCreatedFix -
        Looks for invalid date strings in /mods/originInfo/dateCreated
          fields and attempts to fix them if the --dryRun option
          is NOT set.  Note that --force defaults to TRUE for this command.

      DateFieldFix -
        Looks for invalid date strings in /mods/originInfo/date*
          fields and if found, it removes the field and replaces it with
          a corresponding /mods/note[@type='date'] field, value and, an
          appropriate @displayLabel attribute too. Modified object MODS
          are automatically reordered, self-transformed, and re-indexed
          in Solr.  [Verified 23-Aug-2018]

      DCTransform -
        Applies the specified MODS-to-DC Transform to an object.

      Derivatives -
        Attempts to regenerate ALL derivatives for an existing object.

      Discover -
        Attempts to identify objects that are NOT indexed in Solr.  The
          --force and --repeat options are implied with a default --repeat
          value of 1000.

      HarvestOCR -
        Looks for OCR or FULL_TEXT datastreams in the target objects and,
          if found, it writes the datastream text content to a file named
          [namespace]_[pidn].[ds].txt in
          //STORAGE/LIBRARY/LIBSTU/DG_OCR_Files/Pending_Review/.

      ImportCSV -
        Reads data, one line at at time, from the named --csv CSV data file
        and passes the line to idu_drush_import( ) for processing.

      IndexSolr -
        Attempts to update an object's Solr index.  If the --force option is
        set this command will NOT use Solr to retrieve a list of objects.

      isConstituentOf -
        Changes a literal isConstituentOf value to a proper
          attribute form.

      Last -
        Find the last (largest PID) indexed in Solr for the specified
          namespace and sets the namespace_last variable with it's value.

      List -
        Prints a list of objects matching the PID spec with --repeat,
          --collection, and --contentType options accounted for.

      MakeFamily -
        Converts the specified object to a compound parent and makes the
          original content a new child of that parent.

      MakeSibling -
        Attempts to make the specified object a sibling of an existing
          --sibling PID.

      MigrateObject -
        Migrates an object from its --source collection to --replace.
          [Verified 31-Aug-2018]

      MIMEFix -
        Changes errant object MIME types from --find to --replace.

      PermissionsRelax -
        Applies a 'relaxed' POLICY to an object to make it visible
          to all.  The original POLICY is copied to a new Old_POLICY
          datastream.  If --find is specified the change will be made
          ONLY if the specified string exists in the DC datastream, and
          if both --find and --dsid exist then only the specified DSID
          will be searched.

        Option defaults are:
          dsid=NULL
          find=NULL

      PermissionsRestore -
        Restores an 'Old_POLICY' datastream as POLICY in order to bring
        back an object's original permissions/policy.

      Purge -
        Deletes the object.  If the --force option is set this command
        will NOT use Solr to retrieve a list of objects.
        Be careful with this one! [Verified 30-Apr-2018]

      PurgeDatastream -
        Deletes the --dsid datastream.  Be careful with this one too!
        [Verified 2-May-2018]

      PurgeDSVersions -
        Purges all but the current copy of an existing object's --dsid
        datastream.

      PurgeElements -
        Removes all --xpath matched elements from the specified --dsid
          datastream. [Verified 27-Apr-2018]

      PurgeFamily -
        Delete an object and all of its constituents!

      PurgeInactive -
        Deletes the specified objects if they are in an Inactive or Deleted
          state.

      RemoveDatastreams -
        Removes the named --dsid datastream(s) from an existing object. Note
          that in this context --dsid may contain wildcards like the default
          value of MODS_*. Use --dryRun to count matches only.

      RemoveDuplicateElements -
        Removes duplicate elements from the --dsid datastream of an existing
          object.  Use the --dryRun option to report duplicates only.

      RemoveEmptyCues -
        Removes empty cue elements from the --dsid (default is TRANSCRIPT) datastream
          of an existing oral history object.  Use the --dryRun option to
          report only.

      RenameDatastream -
        Changes the DSID of the named --dsid datastream to one named
          --replace, and if --self is specified, runs the self transform on
          the datastream.

      ReportChildDuplicates -
        Echo the MODS elements of a child object that are duplicated in its
        parent.

      Special -
        Based on ChangeText...this command changes to meet special needs!

        Options: dsid, xpath, dryRun.

      Thumbnail -
        Applies the specified thumbnail image to an object.

      TitleBookPage -
        Changes the label/title of a book page to
          'Parent Title - Page XXX'.

      toIR -
        Migrates an object from any collection to the grinnell:generic
          collection.

      toPHPP -
        Migrates an object from the grinnell:generic collection
          to grinnell:phpp."),

    'options' => array(
      'collection'   => t("Specifies the target collection (should use a wildcard and no colon, like '*test' in lieu of ':test') from which objects will be selected for processing.\n"),
      'contentType'  => t("Specifies the content model (type...should include wildcards like '*image*') of object(s) to be selected for processing.\n"),
      'createdDate'  => t("If set with any portion of a YYYY-mm-ddTHH:ii timestamp string, only objects with a creation date/time matching createdDate* are considered. 2018-12, for example, applies to all objects created in December 2018.  Default is '2' for all objects.\n"),
      'modDate'      => t("If set with any portion of a YYYY-mm-ddTHH:ii timestamp string, only objects with a modified date/time matching modDate* are considered. 2018-12-0, for example, applies to all objects modified on December 1st thru 9th, 2018.  Default is '2' for all objects.\n"),
      'repeat'       => t("Specifies the number of consecutive PIDs following 'first' to be processed.  Default is 'all'.\n"),

      // Options above this line apply to ALL commands.

      'cmodel'    => t("Specifies the target CModel (content model) in the ChangeCompound command.  Defaults to 'islandora/sp_large_image_cmodel'.\n"),
      'compress'  => t("Specifies a file containing Kakadu 'compress' options for the ConvertTIFtoJP2 fix.\n"),
      'contents'  => t("Specifies text to be included in a datastream or XML element.  Default varies by use.\n"),
      'convert'   => t("Specifies a file containing ImageMagick 'convert' options for the ConvertTIFtoJP2 fix.\n"),
      'creator'   => t("Specifies a dc:creator value for the UpdateDC fix.  Default is 'Unknown'.\n"),
      'csv'       => t("Specifies the path of a CSV file to be read. Defaults to public://data.csv.\n"),
      'date'      => t("Specifies a dc:date value for the UpdateDC fix.  Default is 'Unspecified'.\n"),
      'dryRun'    => t("If set, this option instructs the operation to identify and count changes, but not commit then.\n"),
      'dsid'      => t("Specifies the datastream ID, or special 'LABEL', in which to make changes.\n"),
      'find'      => t("Specifies text to be found (and in some cases replaced).\n"),
      'fixName'   => t("If set, this option instructs UpdateDC to ensure that a MODS /name/namePart[not(@*)] element exists.\n"),
      'force'     => t("If set, this option instructs IndexSolr and other commands NOT to use Solr to fetch a list of objects.\n"),
      'hidden'    => t("If set for UpdateDC, this option creates /mods/extension/hidden_creators instead /mods/extension/creators.\n"),
      'ignoreOBJ' => t("If TRUE, this option tells 'ChangeCompound' that no OBJ is required when changing a compoundCModel to another type.\n"),
      'image'     => t("Specifies the path to a jpeg image to be applied by the specified Fix operation.  Defaults to public://ImageNotAvailableTIFF_TN.jpg.\n"),
      'mime'      => t("Specifies the OBJ MIME type of objects to process.  Defaults to 'image/tiff'.\n"),
      'only'      => t("Specifies the ONLY DSID to be regenerated, --only=TECHMD is one example. Defaults to * for all derivatives.\n"),
      'parent'    => t("Specifies the PID of an existing parent object in need of a child.\n"),
      'reorder'   => t("If set, this option instructs SelfTransform to run public://reorder_mods.xsl after the initial self-transform.\n"),
      'reindex'   => t("When used in combination with --force this option forces a pre-emptive re-indexing of objects.\n"),
      'replace'   => t("Specifies the text that 'find' will be replaced with.\n"),
      'retire'    => t("Specifies one collection that target objects are to be removed from when using the CorrectCollections command.\n"),
      'self'      => t("Specifies the path to the self-transform XSL file to be applied. Defaults to public://cleanup_mods_and_reorder.xsl.\n"),
      'sibling'   => t("Specifies the PID of one existing child object in need of a sibling.\n"),
      'source'    => t("Specifies the DC field from which /mods/extension/creators is to be constructed OR,
         in the case of CopyDatastream the PID of the source object.  Default is 'dc:creator'.\n"),
      'substitutionsFile' => t("In place of 'find' and 'replace', this option specifies the path to a 'csv' style file from which 'find, replace' pairs will be read.\n"),
      'title'     => t("Specifies a datastream title (label) or XML element value when needed. Default varies by use.\n"),
      'verbose'   => t("Triggers verbose output in certain commands.\n"),
      'xpath'     => t("Specifies a metadata XPath whenever one is needed. Note that it is wise to put your XPath in double quotes and use single quotes within where necessary.\n"),
      'xsl'       => t("Specifies the path to the MODS-to-DC transform XSL file to be applied. Defaults to public://mods_to_dc_grinnell.xsl.\n"),
    ),
    'examples' => array(
      'Example' => "drush iduF grinnell:10000 ChangeText --dsid=MODS --xpath=\"/mods/relatedItem/identifier[@type='uri']\"",
    ),
    'aliases' => array('iduF'),
  );

  $items['iduUpdate'] = array(
    'description' => t("Attempts to apply the specified 'Update' to one or more existing PIDs."),
    'arguments'   => array(
      'first'  => t("First PID to process.  The namespace portion of this PID defines the default primary namespace.  This parameter may be specified as 'namespace:first-last' in place of the --repeat option."),
      'update' => "Completes the name of the drush_idu_update_NAME function to be called for each processed object.  Available Update functions are:

      Dates -
        Reads the FOXML date entities shown here and maps the data to MODS RecordInfo elements.

        <foxml:property NAME=\"info:fedora/fedora-system:def/model#createdDate\" VALUE=\"2004-12-10T00:21:58.000Z\"/>
        <foxml:property NAME=\"info:fedora/fedora-system:def/view#lastModifiedDate\" VALUE=\"2005-01-20T22:46:07.506Z\"/>",
    ),

    'options' => array(
      'repeat'   => t("Specifies the total number of PIDs, beginning with 'first' to be processed.  Default is 1."),
    ),

    'examples' => array(
      'Example' => "drush iduU grinnell:10000 Dates",
    ),
    'aliases' => array('iduU'),
  );


  $items['iduHandle'] = array(
    'description' => t("Attempts to apply the specified Handle operation to one or more existing PIDs.\n"),
    'arguments'   => array(
      'first'     => t("First PID to process.  The namespace portion of this PID defines the default primary namespace.  This parameter may be specified as 'namespace:first-last' in place of the --repeat option.\n"),
      'handle_op' => "Completes the name of the drush_idu_Handle_NAME function to be called for each processed object.  Available Handle functions are:

           Create - Attempts to create a new handle for an existing object.

           Modify - Attempts to modify an existing handle for an existing object.",
    ),
    'options'  => array(
      'collection'   => t("Specifies the target collection (should include wildcards like '*test*') from which objects will be selected for processing.\n"),
      'contentType'  => t("Specifies the content model (type...should include wildcards like '*image*') of object(s) to be selected for processing.\n"),
      'createdDate' => t("If set with a YYYY-mm-ddTHH:ii:ssZ timestamp string, then only objects created after said local time are considered. Default is 2000-01-01T00:00:00Z.\n"),
      'force'        => t("If specified, handle assignment will proceed even if the mods:identifier[@type='hdl'] element is not compatible with handle_op.\n"),
      'modDate'     => t("If set with a YYYY-mm-ddTHH:ii:ssZ timestamp string, then only objects modified after said local time are considered. Default is 2000-01-01T00:00:00Z.\n"),
      'repeat'       => t("Specifies the total number of PIDs, beginning with 'first' to be processed.  Default is 1.\n"),
    ),
    'examples' => array(
      'Example' => "drush -u 1 iduH grinnell:10000-11000 Create",
    ),
    'aliases' => array('iduH'),
  );

  $items['iduDrupal'] = array(
    'description' => t("Applies the specified operation to the Drupal database or file system."),
    'arguments'   => array(
      'operation' => t('Completes the name of the idu_drush_drupal_OPERATION function to be called.'),
    ),
    'options'     => array( ),
    'examples' => array(
      'Example' => "drush iduD PurgeTempFiles",
    ),
    'aliases' => array('iduD'),
  );

  $items['iduSpecial'] = array(
    'description' => t("Attempts to apply the specified 'Special' operation to a single, existing PID."),
    'arguments'   => array(
      'pidAlpha'  => t('The primary PIDs to be processed.  Example: grinnell:52-100.'),
      'operation' => t('Completes the name of the idu_drush_special_OPERATION function to be called. Available operations are: PrintRI, SingleCompound, FixArtist, FixPrimarySort, FixStatus and GenerateCoversheets.'),
    ),
    'options'     => array(
      'secondary' => t('An optional secondary PID to be used in the process.  Defaults to NULL.'),
      'dryRun'    => t("If set, this option instructs the operation to identify and count changes, but not commit then.\n"),
    ),
    'examples' => array(
      'Example' => "drush iduS grinnell:622 SingleCompound",
    ),
    'aliases' => array('iduS'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function idu_drush_help($command) {
  switch ($command) {
    case 'drush:iduTest':
      return t('Just a temporary test feature.');
    case 'drush:iduFix':
      return t("Attempts to apply the specified 'Fix' to one or more existing PIDs.");
    case 'drush:iduUpdate':
      return t("Attempts to apply the specified 'Update' to one or more existing PIDs.");
    case 'drush:iduSpecial':
      return t("Attempts to apply the specified 'Special' operation to a single existing PID.");
    case 'drush:iduHandle':
      return t("Attempts to apply the specified Handle operation to one or more existing PIDs.");
  }
  return;
}

/**
 * Examine common iduX command arguments/options and set control variables for subsequent processing.
 *
 * @param string $first
 *   The range (or single) PID argument as specified in the iduX command.
 * @param int $last
 *   Returns the 'last' PID value based on the range specification, if any.
 *   Note that $last may or may not be an existing object.
 * @param int $pidn
 *   Returns the integer PID number of 'range'.  Note that $pidn may or may not be an existing object.
 * @param int $range
 *   Returns 'last' - 'first' + 1, the total number of PIDs to address.
 * @param string $namespace
 *   Returns the namespace (without a colon!) associated with the PID numbers returned by the function.
 * @return array
 *   An array of valid PIDs from the specified spec.
 *
 */
function idu_drush_prep($first, &$last, &$pidn, &$range, &$namespace) {

  $after = drush_get_option('modDate', '2');
  $collection = drush_get_option('collection', NULL);
  $contentType = drush_get_option('contentType', NULL);
  $created = drush_get_option('createdDate', NULL);

  $pids = icu_drush_prep($first, $lastPID, $pidn, $range, $namespace, $after, $collection, $contentType, $created);

  // list($ns, $last) = explode(':', $lastPID);
  return $pids;
}

/**
 * Prints a progress bar directly to STDOUT.
 *
 * Call with no values (NULLs) for $current and $range to initialize the progress bar.
 *
 * @param string $label
 *   Label to appear above the bar when initialized.
 * @param int|NULL $current
 *   The current counter (from 0 to $range) in the icg_first to icg_last progression.
 * @param int|NULL $range
 *   The total number of steps in the icg_first to icg_last progression.
 */
function idu_drush_print_progress($label, $current=NULL, $range=NULL) {
  static $green = "\033[1;32m";
  static $white = "\033[0;37m";

  if (is_null($current)) {
    $output = $green . "Progress: $label \n";
    print $output;
    return;
  }

  $ratio = ($current+1) / $range;
  $percentage = floor($ratio * 100) . '%';
  $columns = drush_get_context('DRUSH_COLUMNS', 80);
  // Subtract 10 characters for the percentage, brackets, spaces and arrow.
  $progress_columns = $columns - 10;
  // If ratio is 1 (complete), the > becomes a = to make a full bar.
  $arrow = ($ratio < 1) ? '>' : '=';
  // Print a new line if ratio is 1 (complete). Otherwise, use a CR.
  $line_ending = ($ratio < 1) ? "\r" : "\n";

  // Determine the current length of the progress string.
  $current_length = floor($ratio * $progress_columns);
  $progress_string = str_pad('', $current_length, "=");

  $output  = $green . '[';
  $output .= $progress_string . $arrow;
  $output .= str_pad('', $progress_columns - $current_length);
  $output .= ']';
  $output .= str_pad('', 5 - strlen($percentage)) . $percentage;
  $output .= $line_ending . $white;

  print $output;
}

/**
 * Wrapper for drush_log( ).
 *
 * @param $msg
 * @param $status
 */
function idu_drush_log($msg, $status) {
  $dryRun = drush_get_option('dryRun', FALSE);
  if ($dryRun) { print "$status: $msg\n"; }
  drush_log( $msg, $status );
}

/**
 * Callback function for drush iduFix, alias iduF.
 *
 * This callback runs the prescribed fix for a given PID with an optional --repeat to touch
 * subsequent objects in the same namespace.  Functions called must be of the form...
 *     $count = drush_idu_fix_OPERATION($pid, $findReplace=NULL, $dsid=NULL, $xpath=NULL);
 *
 * @param string $pspec
 *   The list of PIDS to process.
 * @param string $fix
 *   The Fix operation to be performed.  This string completes the name of an existing idu_drush_fix_* function.
 */
function drush_idu_iduFix($pspec, $fix) {
  iduF_validate($fix);
  $verbose = drush_get_option('verbose', FALSE);

  // Get the fix function parameter...
  $function = 'idu_drush_fix_'.$fix;
  if (!function_exists($function)) { drush_log("The specified Fix function '$function' does not exist!", 'error'); }

  $all_pids = array( );
  $pids = array( );
  $pidsFromFile = False;

  // If this is the List or Discover command, save the objects to a .csv file in /tmp.
  if ($fix == 'List' || $fix === 'Discover') {
    $fname = "/tmp/object_list.csv";
    $list = fopen($fname, "w");
    fputcsv($list, array("Function is $fix"));
  }

  // If $pspec is a CSV file path, open the file and read a list of PIDs from it, populate $pids with the object list from the CSV file.

  if (file_exists($pspec)) {
    $count = 0;
    $file = fopen($pspec, 'r');
    while (($line = fgetcsv($file)) !== FALSE) {
      // $line is an array of the csv elements
      foreach ($line as $val) {
        $pids[] = $val;
        $count++;
      }
    }
    fclose($file);
    drush_log("$count PIDs were read from $pspec and processing is ready to commence.  All other PID selection options are ingnored.", 'status');
    $pidsFromFile = True;
    $range = $count;

  } else {     // Not a file path...process the PID spec as usual.

    // If this is a forceable operation called with the --force option, then
    // don't use Solr to retrieve the list of PIDs!
    $forced = array("Discover", "DateCreatedFix", "Last");
    $forceable = array_merge($forced, array(
      "IndexSolr",
      "Purge",
      "ChangeText",
      "Derivatives",
      "PermissionsRelax",
      "List"
    ));

    $force = FALSE;
    $repeat = drush_get_option('repeat', 0);
    $reindex = drush_get_option('reindex', FALSE);

    if (in_array($fix, $forceable)) {
      if (in_array($fix, $forced)) {
        $force = drush_get_option('force', TRUE);
        $repeat = drush_get_option('repeat', 1000);    // repeat at least 1000 times if --repeat is not set
      }
      else {
        $force = drush_get_option('force', FALSE);
        if ($force) {
          $repeat = drush_get_option('repeat', 1000);
        }
      }
    }
    else {
      if (drush_get_option('force', FALSE)) {
        drush_log("The specified Fix function '$function' does not support use of the --force option!", 'warning');
      }
    }

    # If $pspec is ns:last then build $spec as a ns:first-last range.  Echo the PID that is 'last'.
    list($ns, $special) = explode(':', $pspec);
    if ($special === 'last') {
      list($ns, $first) = explode(':', variable_get($ns . '_last', $ns . ':2'));
      $last = $first + $repeat;
      $spec = $ns . ':' . $first . '-' . $last;
      drush_log("ns:last notation used.  '$ns:$first' was identifed as the last indexed PID.", 'status');
    } else {
      list($ns, $spec) = explode(':', $pspec);
      if (strpos($spec, '-') !== false) {
        list($first, $last) = explode('-', $spec);
        if (!$last) {
          $last = $first + $repeat;
        }
      } else {
        $last = $first = $spec;
      }
    }

    $spec = $ns . ':' . $first . '-' . $last;

    // If this retrieve has a valid --force option without --reindex...
    // The next operations depend on Solr so if a numeric range has been specified, make sure all valid objects in that range have been indexed!
    if ($force && $reindex) {
      for ($i = $first; $i <= $last; $i++) {
        $status = idu_drush_update_solr_index("$ns:$i");
      }
    }

    // $preOpts['all_pids'] = idu_drush_prep($spec, $junk, $pidn, $range, $ns, TRUE);  // get all eligible PIDs found in Solr

    if (!$pids = idu_drush_prep($spec, $last, $pidn, $range, $ns, TRUE)) { return; }  // Get PIDs and initialize the loop..

  }

  // Pre-process some options...
  // Check for a --substitutionsFile option, instead of $find and $replace, and process accordingly.
  $findReplace = NULL;
  $file = drush_get_option('substitutionsFile', NULL);
  if (!is_null($file)) {

    // CSV file processing
    if (!$f = fopen($file,"r")) { die("Unable to parse findReplace INI file '$file' for reading!  "); }
    while (!feof($f)) {
      list($find,$replace) = fgetcsv($f);
      $findReplace[trim($find)] = trim($replace);
    }
    fclose($f);

    /* INI file processing
    if (!$findReplace = parse_ini_file($file)) { die("Unable to parse findReplace INI file '$file' for reading!  "); } */

    $msg = count($findReplace)." substitution pair(s) found in '$file'.";
    idu_drush_log($msg, 'status');

  } else {

    $find = drush_get_option('find', NULL);
    $replace = drush_get_option('replace', NULL);
    $findReplace[$find] = $replace;
  }

  // Check for --find and --dsid options.
  if ($find = drush_get_option('find', NULL)) { $preOpts['find'] = $find; }
  if ($dsid = drush_get_option('dsid', NULL)) { $preOpts['dsid'] = $dsid; }

  // Put the pre-fetched options into an array and pass it to the hook function.
  $preOpts['findReplace'] = $findReplace;

  idu_drush_print_progress("iduFix - $fix");
  $count = 0;

  // If there's only one PID... process it.

  if (count($pids) === 1) {
    if ($verbose)  { idu_drush_log("Processing PID $pids[0].", 'status'); }
    $count += call_user_func($function, $pids[0], $preOpts);
    idu_drush_post_to_file("iduFix - $fix: $pids[0] complete.", NULL, 'public://iduFix_progress.log');
    idu_drush_print_progress("iduFix - $fix", 0, 1);

  // If there's no PID number range, process the PID list as-is.

  } else if (!$pidsFromFile && is_null($pidn)) {
    $range = count($pids);
    $i = 0;

    foreach ($pids as $pid => $label) {
      if ($verbose)  { idu_drush_log("Processing PID $pid.", 'status'); }
      list($nsl, $pidn) = explode(':', $pid);
      if ($label && $ns === $nsl) {
        $count += call_user_func($function, $pid, $preOpts);
      }
      idu_drush_print_progress("iduFix - $fix", $i++, $range);
    }

  // Operate on a range of PID numbers.

  } else {
    $i = 0;
    foreach ($pids as $pid) {
      if ($verbose)  { idu_drush_log("Processing PID $pid.", 'status'); }
      $count += call_user_func($function, $pid, $preOpts);
      idu_drush_post_to_file("iduFix - $fix: $pid complete.", NULL, 'public://iduFix_progress.log');
      idu_drush_print_progress("iduFix - $fix", $i++, $range);
    }
  }

  // Done.
  $finish = date('H:i:s');
  idu_drush_log("Completed $count 'iduFix - $fix' operations at $finish. \n", 'status');

  if ($fix === 'Last') {
    $found_last = variable_get($ns.'_last', 'Undefined');
    idu_drush_log("grinnell:last is now '$found_last'.\n", 'status');
  }

  // If this is the List command, close the .csv file in /tmp.
  if (isset($list)) { fclose($list); }

}

//----------------------------------------------------------------------
// Repository wrapper functions...
//----------------------------------------------------------------------

/* Fetch the existing Fedora connection or open a new connection.
*
*/
function _idu_Repository() {
  // Check that the repository is open.  If not, open it.
  if (isset($_SESSION[__FUNCTION__])) {
    $repository = $_SESSION[__FUNCTION__];
  } else {
    $repository = icu_Connect( );
    if (!$repository) {
      drush_log(__FUNCTION__ . " error: No Fedora connection or URL found!", 'error');
      die;
    }
    $_SESSION[__FUNCTION__] = $repository;
    $_SESSION['fedora_url'] = variable_get('islandora_base_url');
  }
  return $repository;
}

/****************************************************************************************************
 * Drush iduH (Handle) command functions follow here in alphabetical order.
*****************************************************************************************************/

/**
 * Callback function for drush iduHandle, alias iduH.
 *
 * This callback runs the prescribed Handle command for a given PID with an optional --repeat
 * to touch subsequent objects in the same namespace.  Functions called must be of the form...
 *     $count = drush_idu_handle_OPERATION($pid);
 *
 * @param string $first
 *   First PID to process.
 * @param string $handle_op
 *   The Handle operation to be performed.  This string completes the name of an existing idu_drush_handle_* function.
 */
function drush_idu_iduHandle($first, $handle_op) {
  // Get the Handle operation parameter...
  $function = 'idu_drush_Handle_'.$handle_op;
  if (!function_exists($function)) { drush_log("The specified Handle function '$function' does not exist!", 'error'); }
  if (!$pids = idu_drush_prep($first, $last, $pidn, $range, $ns, TRUE)) { return; }     // Get the PIDs and initialize the loop...
  idu_drush_print_progress("iduHandle - $handle_op");
  $count = 0;
  // If there's only one PID... process it.
  if (count($pids) === 1) {
    $count += call_user_func($function, $pids[0]);
    idu_drush_post_to_file("iduHandle - $handle_op: $pids[0] complete.", NULL, 'public://iduHandle_progress.log');
    idu_drush_print_progress("iduHandle - $handle_op", 0, 1);
    // Operate on a range of PID numbers.
  } else {
    $i = 0;
    foreach ($pids as $pid) {
      $count += call_user_func($function, $pid);
      idu_drush_post_to_file("iduHandle - $handle_op: $pid complete.", NULL, 'public://iduHandle_progress.log');
      idu_drush_print_progress("iduHandle - $handle_op", $i, $range);
    }
  }
  // Done.
  $finish = date('H:i:s');
  idu_drush_log("Completed $count 'iduHandle - $handle_op' operations at $finish. \n", 'status');

  // If one or more handles were created there should be a batch file to process.  Do that now.
  if ($count > 0) {
    $fileName = 'temporary://'.H_BATCH;
    $logFile = 'temporary://'.H_LOG;
    if (!file_exists($fileName)) { die("Could not find existing batch file $fileName!  "); }
    if (!$file = fopen($fileName, 'r')) { die("Could not open existing batch file $fileName to read!  "); }
    $path = drupal_realpath($fileName);
    $log = drupal_realpath($logFile);

    // Build a call to the batch file and run it.
    $output = array( );
    $command = H_BINDIR."/hdl-genericbatch $path $log.tmp -verbose";
    idu_drush_log("Handle batch command is: '$command'. \n", 'status');
    exec($command, $output, $return_status);

    // Keep a timestamped copy of the log file.
    if (file_exists("$log.tmp")) {
      $newLog = 'temporary://'.H_LOG.time( );
      file_put_contents($newLog, file_get_contents($log));
      file_put_contents($newLog, file_get_contents("$log.tmp"), FILE_APPEND);
      idu_drush_log("Batch call to the Handle System is complete. A copy of the composite log file is stored in '$newLog'.", 'status');

      // Delete the temporary batch command file and log file.
      if (file_exists($fileName)) { unlink($fileName); }
      if (file_exists($logFile)) { unlink($logFile); }
      if (file_exists("$log.tmp")) { unlink("$log.tmp"); }
    } else {
      idu_drush_log("Batch call to the Handle System is complete but no log file was generated?", 'error');
    }
  }

  // Done.
  $finish = date('H:i:s');
  idu_drush_log("Completed $count 'iduHandle - $handle_op' operations at $finish. \n", 'status');
}

/*
 * Drush iduH (Handle) command functions follow here in alphabetical order.
 */

/**
 * idu_drush_Handle_Create($pid);
 *
 * Generates a 'generic batch' command file for the creation/assignment of one Handle for object $pid.
 *
 * The file will be created and run in H_SVRDIR as H_BATCH (see constants.inc in the idu module).
 */
function idu_drush_Handle_Create($pid) {
  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }
  $force = drush_get_option('force', FALSE);
  return idu_handle_create_or_modify($object, 'CREATE', $force);
}

/**
 * idu_drush_Handle_Modify($pid);
 *
 * Generates a 'generic batch' command file to modify and assign the Handle identifier for object $pid.
 *
 * The file will be created and run in H_SVRDIR as H_BATCH (see constants.inc in the idu module).
 */
function idu_drush_Handle_Modify($pid) {
  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }
  $force = drush_get_option('force', FALSE);
  return idu_handle_create_or_modify($object, 'MODIFY', $force);
}



/****************************************************************************************************
 * Drush iduF (Fix) command functions follow here in alphabetical order.
*****************************************************************************************************/

/**
* Drush idu_fix_* function to change the text of elements in the target datastream and xpath.
*
* Invoke this function via iduF and the 'ChangeText' operation parameter.  Something like this...
*
*   drush iduF grinnell:501 ChangeText --substitutionsFile="public://findReplace.csv" --dsid=MODS --xpath="/mods/description/identifier[@type='uri']
*
* Applicable options include the following:
*
*   --substitutionsFile=<file path>  If this option is specified then --find and --replace are ignored and
*       pairs of find, replace CSV strings are pulled instead from the named file.  Default is NULL.
*
*   --find=<string>  If this option is specified, and --substitutionsFile is not, then the specified string
*       is searched for in the target datastreams and replaced with values specified in --replace.
*
*   --replace=<string>  See --find above.
*
*   --dsid=<string>  This option specifies the ID of the datastream to be operated on in each object.
*
*   --xpath=<string>  This option, if present, specifies an xpath used to query the target datastream.  If specified,
*       find => replace operations are restricted to the value returned by the xpath, so only the text() value of
*       the xpath can be changed.  If --xpath is omitted it defaults to NULL and this specifies that the entire
*       datastream be treated as a string and find => replace then operates on it as a string.  You can effectively
*       alter the ‘structure’ of a datastream in this mode so BE VERY CAREFUL!
*
* @param string $pid
*   The PID to operate on.
* @param array $options
*   An array of options.  In this case $options['findReplace'] should carry an associative array
*   with one or more find => replace pairs.
* @return int
*   The number of repair operations performed, or zero if none.
*/
function idu_drush_fix_ChangeText($pid, $options) {

  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }

  // Get the pre-processed find and replace options.
  $findReplace = $options['findReplace'];

  // Check for other options...
  $dsid = drush_get_option('dsid', 'MODS');
  $xpath = drush_get_option('xpath', NULL);
  $dryRun = drush_get_option('dryRun', FALSE);

  $total = $cnt = $count = 0;
  $label = $object->label;

  // If --dsid=LABEL then this find/replace is with respect to the object's Label...
  if ($dsid === 'LABEL') {
    foreach ($findReplace as $from => $to) {
      if ($from === "0xC") {   // if $from is hex 0xC, replace with a space.
        $from = chr(12);
        $to = ' ';
      }

      if (stripos("_$label", $from)) {
        $label = str_replace($from, $to, $label, $cnt);
        $count += $cnt;
        $label = ltrim($label, "_");
      }
    }

  // Otherwise, open the specified datastream and go to work...
  } else if ($ds = $object[$dsid]) {
    $content = $ds->content;

    foreach ($findReplace as $from => $to) {
      if ($from === "0xC") {   // if $from is hex 0xC, replace with a space.
        $from = chr(12);
        $to = ' ';
      }

      if (stripos($content, $from)) {
        if (is_null($xpath)) {
          $content = str_replace($from, $to, $content, $cnt);
          $count += $cnt;
        }
        else {
          $xml = new DOMDocument;
          $xml->loadXML($content);
          $dx = new DOMXPath($xml);
          $results = $dx->query($xpath);
          foreach ($results as $node) {
            $value = (string) $node->nodeValue;
            if (strpos(' ' . $value, $from)) {
              $node->nodeValue = str_replace($from, $to, $value, $cnt);
              $count += $cnt;
              $content = $xml->saveXML();
            }
          }
        }
      }
    }
  }

  if ($count > 0) {
    $msg = "iduFix - ChangeText: $count changes.";
    if ($dryRun) {
      idu_drush_log($msg." NO changes committed for object $pid due to the --dryRun option.", 'status');
      idu_drush_post_to_file($msg." NO changes committed for object $pid due to the --dryRun option.", NULL, 'public://iduFix_progress.log');
    } else if ($dsid === 'LABEL') {
      $total += $count;
      $object->label = $label;
      idu_drush_post_to_file("iduFix - ChangeText: $count changes committed for object $pid's label.", NULL, 'public://iduFix_progress.log');
    } else {
      $total += $count;
      $object[$dsid]->setContentFromString($content);
      idu_drush_post_to_file("iduFix - ChangeText: $count changes committed for object $pid.", NULL, 'public://iduFix_progress.log');
    }
  }

  return $total;
}



 /**
 * Drush idu_fix_* function to regenerate derivatives.
 *
 * Invoke this function using iduF <pids> Derivatives (with --only=TECHMD if you only want new FITS metadata).
 *
 * @param string $pid
 *   The object PID to operate on.
 * @return int
 *   The number of repair operations performed, or zero if none.
 */
 function idu_drush_fix_Derivatives($pid) {
   module_load_include('inc', 'islandora', 'includes/derivatives');

   // For any exception, report and skip this PID
   if (!$object = _idu_Repository()->getObject($pid)) {
     idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
     return 0;
   }

  $only = drush_get_option('only', '*');
  $dsid = drush_get_option('dsid', 'OBJ');

  // Check for --mime option.  If present and the object has no OBJ or the OBJ MIME is not of the correct
  // type, then skip this object.

  $mime = drush_get_option('mime', NULL);
  if (!is_null($mime)) {
    if (!$obj = $object['OBJ']) { return 0; }
    $objMIME = $obj->mimetype;
    if ($mime != $objMIME) { return 0; }
  }

  $count = 0;
  $rels = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
  $nCM = count($rels);
  if ($nCM != 1) { return 0; }
  $cModel = $rels[0]['object']['value'];

  // Create new derivatives.
  $logging_results = islandora_do_derivatives($object, array(
    'source_dsid' => $dsid, 'force' => TRUE ));
  islandora_derivative_logging($logging_results);
  $count++;

  /*
  switch ($cModel) {
    case 'islandora:sp_basic_image' :
      if (module_exists('islandora_basic_image')) {
        if ($only === '*' || $only === "TN") {
          islandora_basic_image_create_thumbnail($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === "MEDIUM_SIZE") {
          islandora_basic_image_create_medium_size($object, TRUE);
          $count++;
        }
      }
      if (module_exists('islandora_fits') && ($only === '*' || $only === "TECHMD")) {
        $fit = islandora_fits_create_techmd($object, TRUE, array('source_dsid'=>'OBJ'));
        if ($fit['success']) { ++$count; }
      }
      break;
    case 'islandora:pageCModel' :
    case 'islandora:sp_large_image_cmodel' :
      if (module_exists('islandora_large_image')) {
        if ($only === '*' || $only === "TN") {
          islandora_large_image_create_tn_derivative($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === "JPG") {
          islandora_large_image_create_jpg_derivative($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === "JP2") {
          islandora_large_image_create_jp2_derivative($object, TRUE);
          $count++;
        }
      }
      if (module_exists('islandora_fits') && ($only === '*' || $only === "TECHMD")) {
        $fit = islandora_fits_create_techmd($object, TRUE, array('source_dsid'=>'OBJ'));
        if ($fit['success']) { ++$count; }
      }
      break;
    case 'islandora:sp_pdf' :
      if (module_exists('islandora_pdf')) {
        if ($only === '*' || $only === "FULL_TEXT") {
          islandora_pdf_add_fulltext_derivative($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === "PREVIEW") {
          islandora_pdf_add_preview_derivative($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === "TN") {
          islandora_pdf_add_tn_derivative($object, TRUE);
          $count++;
        }
      }
      if (module_exists('islandora_fits') && ($only === '*' || $only === "TECHMD")) {
        $fit = islandora_fits_create_techmd($object, TRUE, array('source_dsid'=>'OBJ'));
        if ($fit['success']) { ++$count; }
      }
      break;
    case 'islandora:sp-audioCModel' :
      if (module_exists('islandora_audio')) {
        if ($only === '*' || $only === "TN") {
          islandora_audio_create_thumbnail($object, TRUE);
          $count++;
        }
        if ($only === '*' || $only === 'PROXY_MP3' ) {
          islandora_audio_create_mp3($object, TRUE);
          $count++;
        }
      }
      if (module_exists('islandora_fits') && ($only === '*' || $only === "TECHMD")) {
        $fit = islandora_fits_create_techmd($object, TRUE, array('source_dsid'=>'OBJ'));
        if ($fit['success']) { ++$count; }
      }
      break;
  } */

  return $count;
 }


/**
 * Drush idu_fix_* function to re-index an existing object in Solr.
 *
 * Invoke this function with an iduF process using a command of the form:
 *   drush iduF <pids> IndexSolr
 *
 * There are no extra options.
 *
 * @param string $pid
 *   The PID (from the list of first-to-last) to operate on.
 * @return int
 *   The number of repair operations performed, or zero if none.
 */
function idu_drush_fix_IndexSolr($pid) {

  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }

  // Scan the MODS, DC and FULL_TEXT datastreams looking for non-XML characters.
  $dsids = array("MODS", "DC", "FULL_TEXT");
  foreach ($dsids as $dsid) {
    if ($ds = $object->getDatastream($dsid)) {
      $content = $ds->content;
      if ($x = hasInvalidXml($content)) {
        idu_drush_log(__FUNCTION__ . ": The $dsid datastream of object $pid contains $x invalid characters!", 'error');
      }
    }
  }

  $status = idu_drush_update_solr_index($pid);

  // Check $status for "Number of warnings (see log): 0"...
  if (strpos($status, "Number of warnings (see log): 0") > 0) {
    return 1;
  } else {
    idu_drush_log(__FUNCTION__ . ": Number of Solr warnings was NOT zero for PID $pid!", 'warning');
    if (drush_get_option('verbose', FALSE)) { echo $status; }
    return 0;
  }
}


/**
* Drush idu_fix_* function to migrate (move) an object from one collection to another.
*
* Invoke this function via iduF and the 'MigrateObject' operation parameter.  Something like this...
*
*   drush iduF grinnell:4937 MigrateObject --source=grinnell:pending-review-suppressed --collection=grinnell:test
*
* @param string $pid
*   The PID to operate on.
* @param array $options
*   An array of passed options.
* @return int
*   The number of repair operations performed, or zero if none.
*/
function idu_drush_fix_MigrateObject($pid, $options) {

 // For any exception, report and skip this PID
 if (!$object = _idu_Repository()->getObject($pid)) {
   idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
   return 0;
 }

 if (!$source = drush_get_option('source', False)) {
   idu_drush_log('This command requires a --source option!', 'error');
   return 0;
 }

 if (!$dest = drush_get_option('replace', False)) {
   idu_drush_log('This command requires a --replace (destination) option!', 'error');
   return 0;
 }

 $dryRun = drush_get_option('dryRun', False);

 if ($rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection')) {
   foreach ($rels as $rel) {
     $r = $rel['object']['value'];
     if ($r === $source && !$dryRun) {
       $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
       $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $dest);
       return 1;
     } else {
       idu_drush_log("Object '$pid', part of collection '$r' did NOT migrate due to conflict or --dryRun.", 'warning');
     }
   }
 }
 return 0;
}





/**
 * Drush idu_fix_* function to purge an object (be careful!) from the repository.
 *
 * Invoke this function with an iduF process using a command of the form:
 *   drush iduF <pid> Purge
 *
 * @param string $pid
 *   The PID to be purged.
 * @param array $options
 *   There are NO purge-specific options.
 * @return int
 *   The number of repair operations performed, or zero if none.
 */
function idu_drush_fix_Purge($pid, $options=NULL) {

  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }

  $dryRun = drush_get_option('dryRun', FALSE);

  if (!$dryRun) {
    _idu_Repository( )->purgeObject($pid);
    $result = idu_drush_delete_from_solr_index($pid);
  } else {
    idu_drush_log(__FUNCTION__ . " $pid was found but NOT purged due to use of --dryRun.", 'info');
  }

  return 1;
}


/**
 * Drush idu_fix_* function to remove empty cue elements from a specified
 * object's MODS datastream.
 *
 * Invoke this function with an iduF process using a command of the form:
 *   drush iduF <pids> RemoveEmptyCues
 *
 * @param string $pid
 *   The PID (from the list of first-to-last) to operate on.
 * @return int
 *   The number of empty elements removed, or zero if none.
 */
function idu_drush_fix_RemoveEmptyCues($pid) {

  // For any exception, report and skip this PID
  if (!$object = _idu_Repository()->getObject($pid)) {
    idu_drush_log(__FUNCTION__."($pid) returned NO object!", 'error');
    return 0;
  }

  // Fetch options.
  $dsid = drush_get_option('dsid', 'TRANSCRIPT');
  $dryRun = drush_get_option('dryRun', FALSE);

  // Check that this is an Oral History object.
  $isOH = FALSE;

  $models = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
  foreach ($models as $model) {
    $CModel = $model['object']['value'];
    if ($CModel === 'islandora:oralhistoriesCModel') {
      $isOH = TRUE;
    };
  }
  if (!$isOH) { return 0; }

  $total = $cnt = $count = 0;
  $label = $object->label;

  // Open the TRANSCRIPT datastream and remove any empty cues.
  $ds = $object[$dsid];
  if ($content = $ds->content) {
    $xml = new DOMDocument;
    $xml->loadXML($content);
    $dx = new DOMXPath($xml);
    $parents = $dx->query('/cues');
    foreach ($parents as $parent) {
      $cues = $dx->query('/cues/cue');
      foreach ($cues as $cue) {
        $isEmpty = $cue->childNodes->length === 0;
        if ($isEmpty) {
          $cue->parentNode->removeChild($cue);
          $content = $xml->saveXML();
          $count += 1;
        }
      }
    }
  }

  if ($count > 0 && !$dryRun) {
    $ds->setContentFromString($xml->saveXML());

    try {
      $object->ingestDatastream($ds);
    } catch (Exception $e) {
      echo 'Caught exception: ', $e->getMessage(), "\n";
    }
    $status = idu_drush_update_solr_index($pid);
  }

  return $count;
}


/* Helper functions ---------------------------------------------------------------------------- */

/**
 * Some available drush functions...
 *
 * drush_log('Log an event using drush', 'warning');
 * drush_set_error('Set an error with drush.');
 * dt('Translate strings with drush');
 * drush_print('Print to command line with drush');
 * drush_print_table($rows, TRUE); //print a command line table with drush
 * drush_confirm('Are you sure you want to continue?', $indent = 0); //Add drush confirmation
 *
 */
/**
 * Implements hook_drush_command().
 */
function icu_drush_command( ) {
  $items = array();
  $items['icuConnect'] = array(
    'description' => t('Used to establish a Tuque connection to a named Fedora repository.'),
    'arguments'   => array(
      'fedora'    => t("The Fedora repository URL of the form http://repository.domain.com:8080/fedora."),
    ),
    'examples' => array(
      'Example' => 'drush -u 55 icuC http://repository.grinnell.edu:8080/fedora',
    ),
    'aliases' => array('icuC'),
  );
  return $items;
}
/**
 * Drush callback function for the 'drush icuC' command.
 *
 * @param string $fedora
 *   The Fedora repository path from the sole argument to the icuC command.
 *
 * @return bool|\FedoraRepository
 */
function drush_icu_icuConnect($fedora) {
  // module_load_include('inc', 'icu', 'icu');
  return icu_Connect($fedora);
}
/**
 * Examine common ixxX command arguments/options and return a list of PIDs meeting
 * the specified criteria.
 *
 * @param string $pidSpec
 *   The range (or single) PID argument as specified in the ixxX command.
 * @param int $last
 *   Returns the 'last' PID value based on the range specification, if any.  Note that $last may or may not be an existing object.
 * @param int $pidn
 *   Returns the integer PID number of 'range'.  Note that $pidn may or may not be an existing object.
 * @param int $range
 *   Returns the total number of PIDs to address.
 * @param string $namespace
 *   Returns the namespace (without a colon!) associated with the PID numbers returned by the function.
 * @param null|string $lastOp
 *   The date/time of the last successful operation.  Only objects modified after this date should be processed.
 * @param bool $return
 *   If TRUE, the function returns an array of valid PID numbers ONLY.
 * @return array
 *   An array of valid PIDs from the specified range.
 *
 */
function icu_drush_prep($pidSpec, &$last, &$pidn, &$range, &$namespace, $lastOp='2', $collection=NULL, $contentType=NULL, $created=NULL) {
  // module_load_include('inc', 'icu', 'icu');

  // Initialize some things.
  list($namespace, $pidN) = explode(':', $pidSpec);    // Break $pidSpec into namespace and PID number.
  variable_set('icg_namespace', "$namespace:");
  $local = $lastOp;    // all date/time values are stored in local time
  drush_log(__FUNCTION__." will consider only objects modified with a yyyy-mm-dd local time matching $local*.", 'status');

  // If the $pidN is an asterisk, then we are processing only collection objects.
  if ($pidN === '*' && module_exists('islandora_solr_collection_search')) {
    module_load_include('inc', 'islandora_solr_collection_search');
    $pids = islandora_solr_collection_search_get_collection_list( );
    $range = count($pids);
    drush_log("The --collection, --contentType, --repeat, --modDate and --createdDate options will be ignored so that all $range collection objects will be processed.", 'status');
    return $pids;
  }

  // If $pidN is not an integer...then we have only one PID to process...do it!
  if (intval($pidN) === 0) {
    drush_log("Completed fetch of a single object PID.", 'status');
    $range = 1;
    return array($pidSpec);
  }

  list($begin, $end) = explode('-', $pidN);   // Break $pidN into beginning and end of range, if necessary
  $pidn = intval($begin);

  // Check for the --repeat option or a defined $end to the range.
  if (!empty($end)) {
    $endN = intval($end);
    $repeat = $endN - intval($begin);
    $range = $repeat + 1;
  } else {
    $end = $endN = NULL;
    $r = drush_get_option('repeat', 'all');
    if ($r != 'all') {
      $repeat = intval($r);
      if ($repeat < 0) {
        drush_log("The --repeat option must be 'all', zero or greater!  Forcing --repeat set to 'all'.", 'warning');
        $repeat = 'all';
      } else {
        $range = $repeat + 1;
        $endN = $begin + $repeat;
      }
    } else {
      $repeat = 'all';
    }
  }

  $start = date('H:i:s');
  list($spec, $rest) = explode('-', $pidSpec);
  drush_log("Starting operation for PID '$spec' and --repeat='$repeat' at $start.", 'status');

  if ($collection) { drush_log("PIDs will be selected only within --collection='$collection'.", 'status'); }
  if ($contentType) { drush_log("PIDs will be selected only where --contentType='$contentType'.", 'status'); }
  if ($created) { drush_log("PIDs will be selected only where --createdDate matches '$created*'.", 'status'); }

  // Fetch the specified PIDs and return them.
  drush_log("Fetching all valid object PIDs in the specified range.", 'status');
  $pids = icu_fetch_PIDs($namespace, $pidSpec, $endN, $local, $collection, $contentType, $created);
  $range = count($pids);
  $last = end($pids);

  drush_log("Completed fetch of $range valid object PIDs.", 'status');
  return $pids;
}
/**
 * Prints a Drush progress bar directly to STDOUT.
 *
 * Call with no values (NULLs) for $current and $range to initialize the progress bar.
 *
 * @param string $label
 *   Label to appear above the bar when initialized.
 * @param int|NULL $current
 *   The current counter (from 0 to $range) in the icg_first to icg_last progression.
 * @param int|NULL $range
 *   The total number of steps in the icg_first to icg_last progression.
 */
function icu_drush_print_progress($label, $current=NULL, $range=NULL) {
  static $green = "\033[1;32m";
  static $white = "\033[0;37m";
  if (is_null($current)) {
    $output = $green . "Progress: $label \n";
    print $output;
    return;
  }
  $ratio = ($current+1) / $range;
  $percentage = floor($ratio * 100) . '%';
  $columns = drush_get_context('DRUSH_COLUMNS', 80);
  // Subtract 10 characters for the percentage, brackets, spaces and arrow.
  $progress_columns = $columns - 10;
  // If ratio is 1 (complete), the > becomes a = to make a full bar.
  $arrow = ($ratio < 1) ? '>' : '=';
  // Print a new line if ratio is 1 (complete). Otherwise, use a CR.
  $line_ending = ($ratio < 1) ? "\r" : "\n";
  // Determine the current length of the progress string.
  $current_length = floor($ratio * $progress_columns);
  $progress_string = str_pad('', $current_length, "=");
  $output  = $green . '[';
  $output .= $progress_string . $arrow;
  $output .= str_pad('', $progress_columns - $current_length);
  $output .= ']';
  $output .= str_pad('', 5 - strlen($percentage)) . $percentage;
  $output .= $line_ending . $white;
  print $output;
}

/**
 * Fetch a list of ALL numbered PIDs in a specified namespace.
 *
 * @param $namespace
 *   The namespace of the PIDs to return.
 * @param $start
 *   The beginning PID value (namespace:number).
 * @param null|int $end
 *   The end PID number ONLY.  Defaults to NULL which returns
 *   all available PIDs after $start.
 * @param null|string $onlyIn
 *  Specifies the 'fgs_lastModifiedDate_dt' date for the query to filter objects with.  This must specify a specific millenium, century, decade, year, month, week (9 days), or day in the form: yyyy-mm-dd.  So '2018-12-0' specifies the first 9 days of December, 2018.  Defaults to '2', the start of the millennium (returns ALL objects).
 * @return array|bool
 *   FALSE upon error, or an array of PIDs.
 *
 */
function icu_fetch_PIDs($namespace, $start, $end=NULL, $onlyIn="2", $collection=NULL, $contentType=NULL, $created=NULL) {
  $pids = $pidNums = array( );

  // Check the $namespace and $start arguments.
  list($ns, $startNum) = explode(':', $start);
  $startN = intval($startNum);

  if ($startN < 1 || $namespace != $ns) {
    drush_log(__FUNCTION__." error.  Invalid namespace ($namespace) or start ($start) specified.", 'error');
    return FALSE;
  }

  // The following block of code builds an HTTP Solr request.  As such, it does NOT adhere to user permissions per XACML.
  // It was designed to avoid possible recursion.

  // Build the namespace object query...

  $q = "PID:$namespace*";
  $fq = "&fq=-PID:grinnell-*&fq=fgs_lastModifiedDate_dt:".$onlyIn."*";    // remove "grinnell-*" PIDS and filter by modified date

  if ($collection) { $fq .= "&fq=RELS_EXT_isMemberOfCollection_uri_s:".$collection; }
  if ($contentType) { $fq .= "&fq=RELS_EXT_hasModel_uri_s:".$contentType; }
  if ($created) { $fq .= "&fq=fgs_createdDate_dt:".$created."*"; }   // filter by created date

  $url = variable_get('islandora_solr_url', 'solr:8080/solr');
  $fl = 'PID';

  if (preg_match('/^http:\/\/\w.*/', $url) == 1) {
    $request = $url . '/select?q=' . $q . $fq . '&fl=' . $fl . '&wt=php&rows=999999';
  } else {
    $request = 'http://' . $url . '/select?q=' . $q . $fq . '&fl=' . $fl . '&wt=php&rows=999999';
  }

  // Build and execute the Solr query.
  // Solr query code from http://wiki.apache.org/solr/SolPHP
  // $request = "http://solr:8080/solr/select?q=PID:grinnell*&fq=fgs_lastModifiedDate_dt:2019*&fl=PID&wt=php&rows=999999";

  $code = file_get_contents($request);
  if ($verbose = variable_get('verbose')) {
    drush_log("The Solr PID query return code is:\n$code.", 'status');
  }

  if ($code) {
    eval("\$result = " . $code . ";");
    if ($result) {
      $o = $result['response']['docs'];
      foreach ($o as $a) {
        $pid = $a['PID'];
        list($ns, $pidn) = explode(':',$pid);
        $pidN = intval($pidn);
        if ($pidN >= $startN && (is_null($end) || $pidN <= $end)) { $pidNums[] = $pidN; }  // save only numeric PID numbers
      }
    }
    asort($pidNums);                   // sort by PID number
    foreach ($pidNums as $pidN) {
      $pids[] = $ns.':'.$pidN;         // return PIDs in sorted order
    }
    return $pids;
  }
  return FALSE;
}

/**
 * Return the proper MIME type for an OBJ datastream based on content model.  Some have * because they can vary.
 *
 * @return mixed
 */
function icu_OBJMimes( ) {
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
 * Open a connection to the Fedora repository as the identified Drupal user.
 */
function icu_Connect( ) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  module_load_include('inc', 'islandora', 'includes/tuque_wrapper');

  global $user;
  if (!$connection = islandora_get_tuque_connection($user)) {
    drush_log(__FUNCTION__." error: Unable to verify a repository connection!", 'error');
    return FALSE;
  }

  $connection->reuseConnection = TRUE;
  $repository = $connection->repository;
  drush_log(__FUNCTION__.": Connection to Fedora repository as '{$repository->ri->connection->username}' is complete.", 'status');

  return $repository;
}

/**
 * Counts invalid XML characters in a string.
 *
 * @param $value
 *   The XML content string.
 * @return bool|int
 *   False or a count of invalid characters in $value.
 */
function hasInvalidXml($value) {
  // $ret = "";
  $x10FFFF = hexdec('10FFFF');
  $ret = 0;

  if (empty($value)) { return $ret; }

  $length = strlen($value);
  for ($i=0; $i < $length; $i++) {
    $current = ord($value{$i});
    if (($current === 9) ||
      ($current === 10) ||
      ($current === 13) ||
      (($current >= 32) && ($current <= 55295)) ||
      (($current >= 57344) && ($current <= 65533)) ||
      (($current >= 65536) && ($current <= $x10FFFF))) {
      // $ret .= chr($current);
    } else {
      // $ret .= " ";
      $ret++;
    }
  }
  return $ret;
}

/**
 * Does just what it says, for one $pid.
 *
 * @param $pid
 * @return bool|mixed
 */
function idu_drush_update_solr_index( $pid ) {
  module_load_include('inc', 'idu', 'includes/idu');

  // Attempt to update the FedoraGSearch index first.
  $curl = variable_get('islandora_base_url') . 'gsearch/rest/rest?operation=updateIndex&action=fromPid&value='.urlencode($pid);
  $result = curl_query( $curl );

  return $result;
}

/**
 * Does just what it says, for one $pid.
 *
 * @param $pid
 * @return bool|mixed
 */
function idu_drush_delete_from_solr_index( $pid ) {
  module_load_include('inc', 'idu', 'includes/idu');

  // Attempt to delete from the FedoraGSearch index first.
  $curl = variable_get('islandora_base_url') . 'gsearch/rest/rest?operation=updateIndex&action=deletePid&value='.urlencode($pid);
  $result = curl_query( $curl );

  return $result;
}

/**
 * Write a comment and/or follow-up iduF (or similar command) to a script for use later on.
 *
 * @param string $comment
 * @param string $command
 */
function idu_drush_post_to_file($comment, $command, $path=NULL) {

  // if $path is NULL... Fetch the --script option.
  if (is_null($path)) {
    $script = drush_get_option('script', 'public://iduPendingOps.sh');
  } else {
    $script = $path;
  }

  // If there is a comment, prepend $comment with # and a timestamp.
  if (!is_null($comment)) {
    file_put_contents($script, "# $comment [" . date('Y-m-d H:i:s') . "] \n", FILE_APPEND);
  }

  // Now output the command as-is.
  if (!is_null($command)) {
    file_put_contents($script, "$command\n", FILE_APPEND);
  }
}

/*
@TODO... Fix this!
*/

//do this until we expose these in a module or library

@include_once 'sites/all/libraries/tuque/Datastream.php';
@include_once 'sites/all/libraries/tuque/FedoraApi.php';
@include_once 'sites/all/libraries/tuque/FedoraApiSerializer.php';
@include_once 'sites/all/libraries/tuque/Object.php';
@include_once 'sites/all/libraries/tuque/RepositoryConnection.php';
@include_once 'sites/all/libraries/tuque/Cache.php';
@include_once 'sites/all/libraries/tuque/RepositoryException.php';
@include_once 'sites/all/libraries/tuque/Repository.php';
@include_once 'sites/all/libraries/tuque/FedoraRelationships.php';
