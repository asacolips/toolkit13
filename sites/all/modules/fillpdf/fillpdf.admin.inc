<?php

/**
 * @file
 * Allows mappings of PDFs to site content
 */

define('FILLPDF_REPLACEMENTS_DESCRIPTION', t("<p>Tokens, such as those from CCK, sometimes output values that need additional
  processing prior to being sent to the PDF. A common example is when a key within a CCK <em>Allowed values</em>
  configuration does not match the field name or option value in the PDF that you would like to be selected but you
  do not want to change the <em>Allowed values</em> key.</p><p>This field will replace any matching values with the
  replacements you specify. Specify <strong>one replacement per line</strong> in the format
  <em>original value|replacement value</em>. For example, <em>yes|Y</em> will fill the PDF with
  <strong><em>Y</em></strong> anywhere that <strong><em>yes</em></strong> would have originally
  been used. <p>Note that omitting the <em>replacement value</em> will replace <em>original value</em>
  with a blank, essentially erasing it.</p>"));

/* ---------------- Configuration --------------------*/

/**
 * Settings form for user to place API Key
 */
function fillpdf_settings($form, &$form_state) {
  $fillpdf_service = variable_get('fillpdf_service');

  // Assemble service options. Warning messages will be added next as needed.
  $options = array(
    'pdftk' => t('Use locally-installed pdftk: You will need a VPS or a dedicated server so you can install pdftk: (!see_documentation).', array('!see_documentation' => l(t('see documentation'),  'http://drupal.org/documentation/modules/fillpdf'))),
    'local' => t('Use locally-installed PHP/JavaBridge: You will need a VPS or dedicated server so you can deploy PHP/JavaBridge on Apache Tomcat: (!see_documentation).', array('!see_documentation' => l(t('see documentation'),  'http://drupal.org/documentation/modules/fillpdf'))),
    'remote' => t('Use Fill PDF Service: Sign up for <a href="https://fillpdf-service.com">Fill PDF Service</a>.'),
  );

  // Check for JavaBridge.
  if (!(file_exists(drupal_get_path('module', 'fillpdf') . '/lib/JavaBridge/java/Java.inc'))) {
    $options['local'] .= '<div class="messages warning">' . t('JavaBridge is not installed locally.') . '</div>';
  }

  // Check for pdftk.
  $status = fillpdf_pdftk_check(fillpdf_pdftk_path());
  if ($status === FALSE) {
    $options['pdftk'] .= '<div class="messages warning">' . t('pdftk is not properly installed.') . '</div>';
  }

  $form['fillpdf_service'] = array(
    '#type' => 'radios',
    '#title' => t('PDF-filling service'),
    '#description' => t('This module requires the use of one of several external PDF manipulation tools. Choose the service you would like to use.'),
    '#default_value' => $fillpdf_service,
    '#options' => $options,
  );
  $form['remote'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Fill PDF Service'),
    '#collapsible' => TRUE,
    '#collapsed' => $fillpdf_service !== 'remote',
  );
  $form['remote']['fillpdf_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => variable_get('fillpdf_api_key', ''),
    '#description' => t('You need to sign up for an API key at <a href="https://fillpdf-service.com">Fill PDF Service</a>'),
  );
  $form['remote']['fillpdf_remote_protocol'] = array(
    '#type' => 'radios',
    '#title' => t('Use HTTPS?'),
    '#description' => t('It is recommended to select <em>Use HTTPS</em> for this option. Doing so will help prevent
      sensitive information in your PDFs from being intercepted in transit between your server and the remote service.'),
    '#default_value' => variable_get('fillpdf_remote_protocol', 'https'),
    '#options' => array(
      'https' => t('Use HTTPS'),
      'http' => t('Do not use HTTPS'),
      ),
  );
  $form['fillpdf_pdftk_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Configure path to pdftk'),
    '#description' => t("If Fill PDF is not detecting your pdftk installation, you can specify the full path to the program here. Include the program name as well. For example, <em>/usr/bin/pdftk</em> is a valid value. You can almost always leave this field blank. If you should set it, you'll probably know."),
    '#default_value' => variable_get('fillpdf_pdftk_path'),
  );

  drupal_add_js(drupal_get_path('module', 'fillpdf') . '/fillpdf.js');

  return system_settings_form($form);
}

function fillpdf_settings_validate($form, &$form_state) {
  if ($form_state['values']['fillpdf_pdftk_path']) {
    $status = fillpdf_pdftk_check($form_state['values']['fillpdf_pdftk_path']);
    if ($status === FALSE) {
      form_set_error('fillpdf_pdftk_path', t('The path you have entered for
      <em>pdftk</em> is invalid. Please enter a valid path.'));
    }
  }
}


/* ---------------- Form Create --------------------*/

/**
 * Manage your existing forms, or upload a new one
 */
function fillpdf_forms_admin($form, &$form_state) {
  $result = db_query("SELECT title, url, fid FROM {fillpdf_forms} ORDER BY title");
  $header = array(
    t('Title'),
    t('Location'),
    array(
      'data' => t('Operations'),
      'colspan' => '4',
    ),
  );
  $rows = array();
  foreach ($result as $pdf_form) {
    $row = array(
      check_plain($pdf_form->title),
      check_plain($pdf_form->url),
      l(t('Edit'), "admin/structure/fillpdf/$pdf_form->fid"),
      l(t('Delete'), "admin/structure/fillpdf/$pdf_form->fid/delete"),
      l(t('Export field mappings'), "admin/structure/fillpdf/$pdf_form->fid/export"),
      l(t('Import field mappings'), "admin/structure/fillpdf/$pdf_form->fid/import"),
    );
    $rows[] = $row;
  }

  $form['existing_forms'] = array(
    '#markup' => theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'fillpdf'))),
  );

  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $form['upload_pdf'] = array(
    '#type' => 'file',
    '#title' => 'Upload',
    '#description' => 'Upload a PDF template to create a new form',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
    '#weight' => 15,
  );

  return $form;
}

/**
 * Makes sure the Upload was provided (want to validate .pdf here too)
 */
function fillpdf_forms_admin_validate($form, &$form_state) {
  // uploading anything?
  $file = $_FILES['files']['name']['upload_pdf'];
  if (!$file) {
    form_set_error('url', t('A PDF must be provided.'));
  }

  $validate_file = _fillpdf_validate_upload($file);
  if (isset($validate_file['#error'])) {
    form_set_error('url', $validate_file['#message']);
  }
}

function _fillpdf_validate_upload($file) {
  // from includes/file.inc, line 634, but can't use that function because file not an object yet
  if (!preg_match('/\.pdf$/i', $file)) {
    return array(
      '#error' => TRUE,
      '#message' => t('Only PDF files are allowed'),
    );
  }

  // directory exist or writeable?
  $dir = file_build_uri('fillpdf');
  file_prepare_directory($dir, FILE_CREATE_DIRECTORY);

  return TRUE;
}

/**
 * Creates a new Form from the uploaded PDF, including parsed fields
 */
function fillpdf_forms_admin_submit($form, &$form_state) {
  $fid = _fillpdf_save_upload('upload_pdf');
  if (is_numeric($fid)) {
    $form_state['redirect'] = "admin/structure/fillpdf/$fid";
  }
}

function _fillpdf_save_upload($form_key, $fid = NULL) {
  $dir = file_build_uri('fillpdf');
  // $validators not working, so I just checked manually
  // in fillpdf_forms_validate()
  $validators = array('file_validate_extensions' => array('pdf'));
  if ($file = file_save_upload($form_key, $validators, $dir, FILE_EXISTS_RENAME)) {
    drupal_set_message(t('<strong>@filename</strong> was successfully uploaded.', array('@filename' => $file->filename)));
    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    // Does this file already exist in {fillpdf_forms}?
    // If so, don't re-insert it.
    if (isset($fid) === FALSE) {
      db_insert('fillpdf_forms')
        ->fields(array(
          'fid' => $file->fid,
          'title' => $file->filename,
          'url' => $file->uri,
        ))
        ->execute();
      $fid = $file->fid;
    }
    else {
      db_update('fillpdf_forms')
        ->fields(array(
          'url' => $file->uri,
        ))
        ->condition('fid', $fid)
        ->execute();
    }
    fillpdf_parse_pdf($fid);
    return $fid;
  }
  else {
    // commented out because even though error if file doesn't upload right, not error if they dont' upload a file (& this is still triggered)
    drupal_set_message(t('Error saving file to @dir', array('@dir' => $dir)), 'error');
  }
}

/* ---------------- Form Edit --------------------*/

/**
 * Edit existing PDF form
 */
function fillpdf_form_edit($form, &$form_state, $fid) {
  $pdf_form = db_query("SELECT * FROM {fillpdf_forms} WHERE fid = :fid", array(':fid' => $fid))->fetch();

  if ($pdf_form === FALSE) {
    drupal_set_message(t('Non-existent Fill PDF Form ID.'), 'error');
    drupal_not_found();
    drupal_exit();
  }

  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#maxlength' => 127,
    '#default_value' => $pdf_form->title,
    '#required' => TRUE,
    '#description' => t('Enter a title for this mapping configuration.
      This will also be used for deciding the filename of your PDF. <strong>This
      field supports tokens.</strong>'),
  );

  $form['title_tokens_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tokens',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['title_tokens_fieldset']['tokens'] = _fillpdf_admin_token_form();
  $form['default_nid'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Node ID'),
    '#description' => t('When filling a PDF, use this node for the data source if no node is specified in the Fill PDF URL.'),
    '#maxlength' => 10,
    '#default_value' => $pdf_form->default_nid,
  );

  // @@TODO:
  // They can upload a PDF any time, but fields will only be generated on add.  Don't want to purge existing fields,
  // however a user might have accidently uploaded an old template and discover much later (if it's substantially different, just
  // create a new Form
  $form['pdf_info'] = array(
    '#type' => 'fieldset',
    '#title' => 'PDF Form information',
    '#collapsed' => TRUE,
  );
  $form['pdf_info']['submitted_pdf'] = array(
    '#type' => 'item',
    '#title' => t('Uploaded PDF'),
    '#description' => $pdf_form->url,
  );
  $form['pdf_info']['upload_pdf'] = array(
    '#type' => 'file',
    '#title' => 'Update PDF template',
    '#description' => 'Update the PDF template used by this form',
  );
  $form['pdf_info']['sample_populate'] = array(
    '#type' => 'item',
    '#title' => 'Sample PDF',
    '#description' => l(t('See which fields are which in this PDF.'), fillpdf_pdf_link($fid, NULL, NULL, TRUE)) . '<br />' .
    t('If you have set a custom path on this PDF, the sample will be saved there silently.'),
  );
  if (!empty($pdf_form->default_nid)) {
    $form['pdf_info']['populate_default'] = array(
      '#type' => 'item',
      '#title' => 'Fill PDF from default node',
      '#description' => l(t('Download this PDF filled with data from the default node (@node).', array('@node' => $pdf_form->default_nid)), fillpdf_pdf_link($fid)) . '<br />' .
      t('If you have set a custom path on this PDF, the sample will be saved there silently.'),
    );
  }
  $form['pdf_info']['form_id'] = array(
    '#type' => 'item',
    '#title' => 'Form Info',
    '#description' => "Form ID: [$fid].  Populate this form with node IDs, such as /fillpdf?fid=$fid&nid=10<br/>",
  );
  $form['extra'] = array(
    '#type' => 'fieldset',
    '#title' => t('Additional PDF settings'),
    '#collapsible' => TRUE,
    '#collapsed' => ($pdf_form->destination_path || $pdf_form->replacements ? FALSE : TRUE),
  );
  $form['extra']['destination_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom path for generated PDFs'),
    '#description' => t("<p>By default, filled PDFs are not saved to disk; they are simply sent
      directly to the browser for download. Enter a path here to change this behavior (tokens allowed).
      <strong>Warning! Unless you include the &download=1 flag in the Fill PDF URL, PDFs will only
      be saved to disk <em>and won't</em> be sent to the browser as well.</strong></p><p>The path
      you specify must be in one of the following two formats:<br />
        <ul>
          <li><em>path/to/directory</em> (path will be treated as relative to
          your <em>files</em> directory)</li>
          <li><em>/absolute/path/to/directory</em> (path will be treated as relative to your entire
          filesystem)</li>
        </ul>
      Note that, in both cases, you are responsible for ensuring that the user under which PHP is running can write to this path. Do not include a trailing slash.</p>"),
    '#maxlength' => 255,
    '#default_value' => $pdf_form->destination_path,
  );
  $form['extra']['destination_redirect'] = array(
    '#type' => 'checkbox',
    '#title' => t('Redirect directly to PDF'),
    '#description' => t("<strong>This setting is applicable only if a <em>Custom path for generated PDFs</em> is set.</strong> Instead of redirecting your visitors to the front page, it will redirect them directly to the PDF. However, if you pass Drupal's <em>destination</em> query string parameter, that will override this setting."),
    '#default_value' => $pdf_form->destination_redirect,
  );
  $form['extra']['tokens_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tokens',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['extra']['tokens_fieldset']['tokens'] = _fillpdf_admin_token_form();
  $form['extra']['replacements'] = array(
    '#type' => 'textarea',
    '#title' => t('Transform filled PDF field values'),
    '#wysiwyg' => FALSE,
    '#description' => FILLPDF_REPLACEMENTS_DESCRIPTION,
    '#default_value' => $pdf_form->replacements,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/fillpdf',
  );
  $form['#pdf_form'] = $pdf_form;

  // @@TODO: order by weight, and add dragable ala http://www.computerminds.co.uk/quick-guide-using-drupal-add-tabledrag-and-enjoying-jquery-drag-and-drop-loveliness
  $q = db_query('SELECT * FROM {fillpdf_fields} WHERE fid = :fid', array(':fid' => $fid));
  $header = array(t('Label'), t('PDF-field key'), t('Value'), t('Transformed'), array(
    'data' => t('Operations'),
    'colspan' => 2,
  ));
  $rows = array();
  foreach ($q as $field) {
    $row = array(
      check_plain($field->label), // editable
      check_plain($field->pdf_key),
      $field->value, // editable, expandable
      ($field->replacements ? 'Yes' : 'No'),
      // rawurlencode() is needed twice to fully protect "/". Otherwise, "/" is
      // taken as a separator when looking for a match in hook_menu().
      l(t('Edit'), "admin/structure/fillpdf/$fid/edit/" . rawurlencode(rawurlencode($field->pdf_key))),
    );
    $rows[] = $row;
  }

  $form['existing_fields'] = array(
    '#markup' => '<br/><br/>' . theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'fillpdf_fields'))),
  );

  $form['export_fields'] = array(
    '#prefix' => '<div>',
    '#markup' => l(t('Export these field mappings'), "admin/structure/fillpdf/{$pdf_form->fid}/export"),
    '#suffix' => '</div>',
  );

  $form['import_fields'] = array(
    '#prefix' => '<div>',
    '#markup' => l(t('Import a previous export into this PDF'), "admin/structure/fillpdf/{$pdf_form->fid}/import"),
    '#suffix' => '</div>',
  );

  return $form;
}

function fillpdf_form_edit_validate($form, &$form_state) {
  if ($file = $_FILES['files']['name']['upload_pdf']) {
    $validate_file = _fillpdf_validate_upload($file);
    if (isset($validate_file['#error'])) {
      form_set_error('url', $validate_file['#message']);
    }
  }
}

/**
 * Submit Edit or Delete for existing PDF form
 */
function fillpdf_form_edit_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Delete')) {
    $form_state['redirect'] = "admin/structure/fillpdf/{$form['#pdf_form']->fid}/delete";
    return;
  }
  else {
    db_update('fillpdf_forms')
  ->fields(array(
      'title' => $form_state['values']['title'],
      'default_nid' => (int) $form_state['values']['default_nid'] > 0 ? (int) $form_state['values']['default_nid'] : NULL,
      'destination_path' => $form_state['values']['destination_path'],
      'replacements' => $form_state['values']['replacements'],
      'destination_redirect' => $form_state['values']['destination_redirect'],
    ))
  ->condition('fid', $form['#pdf_form']->fid)
  ->execute();
  if ($file = $_FILES['files']['name']['upload_pdf']) {
    // Export the current field mappings to a variable
    $mappings = fillpdf_generate_mappings($form['#pdf_form'], TRUE);

    // Save the uploaded file; this also re-parses it
    _fillpdf_save_upload('upload_pdf', $form['#pdf_form']->fid);

    // Import the ones we just saved. This ensures there are
    // orphaned mappings.
    drupal_set_message(t('Your previous field mappings have been transferred to the new PDF template you uploaded. Review the messages below to make sure the results are what you expected.'));
    fillpdf_import_mappings($form['#pdf_form'], $mappings);
  }

  $form_state['redirect'] = "admin/structure/fillpdf/{$form['#pdf_form']->fid}";
  drupal_set_message(t('Successfully updated form.'));
  }
}

/**
 * Delete form confirmation
 */
function fillpdf_form_delete_confirm($form, &$form_state, $pdf_form) {
  if (is_numeric(arg(3))) {
    $pdf_form = db_query("SELECT * FROM {fillpdf_forms} WHERE fid = :fid", array(':fid' => arg(3)))->fetch();
  }
  if (!$pdf_form) {
    drupal_not_found();
    drupal_exit();
  }

  $form['#pdf_form'] = $pdf_form;
  return confirm_form($form,
    t('Are you sure you want to delete the form %title?', array('%title' => $pdf_form->title)),
    'admin/structure/fillpdf',
    t('Deleting a form will delete all the fields you created in it. This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * Delete form submit
 */
function fillpdf_form_delete_confirm_submit($form, &$form_state) {
  db_delete('fillpdf_fields')
  ->condition('fid', $form['#pdf_form']->fid)
  ->execute();
  db_delete('fillpdf_forms')
  ->condition('fid', $form['#pdf_form']->fid)
  ->execute();
  drupal_set_message(t('Your form has been deleted.'));
  $form_state['redirect'] = 'admin/structure/fillpdf';
}

/**
 * Export an importable array of PDF field key -> Label, Value mappings.
 * The array key is the PDF field key and the value is another array containing the label and
 * the value (properly keyed).
 *
 * @param mixed $pdf_form The FillPDF form ID.
 */
function fillpdf_form_export($pdf_form) {
  $fillpdf_code = fillpdf_generate_mappings($pdf_form);
  return drupal_get_form('fillpdf_export_form', $fillpdf_code);
}

function fillpdf_generate_mappings($pdf_form, $skip_encoding = FALSE) {
  if (is_numeric($pdf_form)) {
    $fid = db_query("SELECT * FROM {fillpdf_forms} WHERE fid = :fid", array(':fid' => $pdf_form))->fetch();
  }
  else {
    $fid = $pdf_form;
  }
  if (!$fid) {
    drupal_not_found();
    drupal_exit();
  }
  $fields = db_query('SELECT * FROM {fillpdf_fields} WHERE fid = :fid', array(':fid' => $fid->fid));
  $export_array = array();
  foreach ($fields as $field) {
    $export_array[$field->pdf_key] = array(
      'label' => $field->label,
      'value' => $field->value,
      'replacements' => $field->replacements,
    );
  }
  return ($skip_encoding === FALSE ? json_encode($export_array) : $export_array);
}

/**
 * Form for exporting Fill PDF field mappings
 */
function fillpdf_export_form($form, $form_state, $code) {
  $form = array();
  $form['export'] = array(
    '#type' => 'textarea',
    '#title' => t('Fill PDF Form Mappings'),
    '#default_value' => $code,
    '#rows' => 30,
    '#description' => t('Copy this code and then on the site you want to import to, go to the Edit page for the Fill PDF form for which you want to import these mappings, and paste it in there.'),
    '#attributes' => array(
      'style' => 'width: 97%;',
    ),
  );
  return $form;
}

/**
 * Based loosely on Node Export's import form. Import the code and configure the DB settings.
 *
 * @param mixed $form_state
 * @param mixed $pdf_form
 */
function fillpdf_form_import_form($form, &$form_state, $pdf_form) {
  if (is_numeric($pdf_form)) {
    $fid = db_query("SELECT * FROM {fillpdf_forms} WHERE fid = :fid", array(':fid' => $pdf_form))->fetch();
  }
  if (!$fid) {
    drupal_not_found();
    drupal_exit();
  }
  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $fid->fid,
  );
  $form['paste'] = array(
    '#type' => 'fieldset',
    '#title' => t('Paste code'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['paste']['code'] = array(
    '#type' => 'textarea',
    '#default_value' => '',
    '#rows' => 30,
    '#description' => t('Cut and paste the results of a <em>Fill PDF Field Mappings export</em> here.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#suffix' => l(t('Reset the form'), $_GET['q']),
  );
  return $form;
}

/**
 * Check the syntax of the code the user is trying to import
 */
function fillpdf_form_import_form_validate($form, &$form_state) {
  $mappings = json_decode($form_state['values']['code'], TRUE);
  if (empty($mappings) || !is_array($mappings)) {
    form_set_error('code', t('There was a problem processing your Fill PDF Field Mappings code. Please do a fresh export from the source and try pasting it again.'));
  }
  else {
    $form_state['values']['mappings'] = $mappings;
  }
}

/**
 * Perform the import.
 */
function fillpdf_form_import_form_submit($form, &$form_state) {
  $pdf_form = new stdClass();
  $pdf_form->fid = $form_state['values']['fid'];
  $mappings = $form_state['values']['mappings'];

  fillpdf_import_mappings($pdf_form, $mappings);
  $form_state['redirect'] = "admin/structure/fillpdf/{$pdf_form->fid}";
}

/**
 * Import an array of decoded Fill PDF mappings.
 * For the format,
 * @see fillpdf_generate_mappings()
 */
function fillpdf_import_mappings($pdf_form, $mappings) {
  $fields = fillpdf_get_fields($pdf_form->fid);
  $field_keys = array_keys($fields);
  // Process the mappings
  foreach ($mappings as $pdf_key => $field_settings) {
    if (in_array($pdf_key, $field_keys)) {
      $field_settings = (object) $field_settings;
      $field_settings->pdf_key = $pdf_key;
      fillpdf_update_field($pdf_form, $field_settings, $pdf_key);
    }
    else {
      drupal_set_message(t('Your code contained field mappings for the PDF field key <em>@pdf_key</em>, but it does not exist on this form. Therefore, it was ignored.', array('@pdf_key' => $pdf_key)), 'warning');
    }
  }
  drupal_set_message(t('Successfully imported matching PDF field keys. If any field mappings failed to import, they are listed above.'));
}

/* ---------------- Fields Edit --------------------*/


/**
 * Retrieve a field from a PDF for use in editing form.
 */
function fillpdf_field($op, $fid, $pdf_key = NULL) {
  if (is_numeric($fid)) {
    $pdf_form = db_query("SELECT * FROM {fillpdf_forms} WHERE fid = :fid", array(':fid' => $fid))->fetch();
  }
  if (!$pdf_form) {
    drupal_not_found();
    drupal_exit();
  }

  if ($op == 'add') {
    drupal_set_title($pdf_form->title);
  }
  elseif ($op != 'edit') {
    return fillpdf_form_overview($pdf_form);
  }
  elseif ($pdf_key) {
    $pdf_key = rawurldecode(rawurldecode($pdf_key));
    $field = db_query("SELECT * FROM {fillpdf_fields} WHERE pdf_key = :pdf_key AND fid = :fid", array(':pdf_key' => $pdf_key, ':fid' => $fid))->fetch();
    if (!$field) {
      drupal_not_found();
      drupal_exit();
    }
    if (!empty($field->label)) {
      $title = $field->label;
    }
    else {
      $title = $field->pdf_key;
    }
    drupal_set_title(t('Edit field mapping for %field_title', array('%field_title' => $title)), PASS_THROUGH);
  }

  return drupal_get_form('fillpdf_field_edit', $pdf_form, $field);
}

/**
 * Edit a single field.
 */
function fillpdf_field_edit($form, &$form_state, $pdf_form, $field) {
  $form['pdf_key'] = array(
    '#type' => 'item',
    '#title' => t('PDF Key'),
    '#markup' => $field->pdf_key,
    '#description' => t('The field key contained in the PDF form.'),
    '#weight' => 0,
  );
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#maxlength' => 255,
    '#default_value' => $field->label,
    '#description' => t('An optional label to help you identify the field.'),
    '#weight' => 1,
  );
  $form['value'] = array(
    '#type' => 'textarea',
    '#title' => t('Value'),
    '#default_value' => $field->value,
    '#description' => t('<p>The content that will populate this field when the PDF is printed/saved.
      This content pulls data via tokens, see below for available tokens.</p>
      <p>To fill this field with an image, use the special pseudo-token <strong>[stamp:field_name]</strong>.
      If your <em>Image</em> field is named <em>field_image</em>, then you would use
      <strong>[stamp:field_image]</strong>.</p>'),
    '#weight' => 4,
  );
  $form['tokens_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tokens',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 5,
  );
  $form['tokens_fieldset']['tokens'] = array(
    '#theme' => 'token_tree',
    '#token_types' => array('node', 'webform'),
  );
  $form['extra'] = array(
    '#type' => 'fieldset',
    '#title' => t('Transform values on this field'),
    '#collapsible' => TRUE,
    '#collapsed' => ($field->replacements ? FALSE : TRUE),
    '#weight' => 6,
  );

  $form['extra']['replacements'] = array(
    '#type' => 'textarea',
    '#wysiwyg' => FALSE,
    '#description' => FILLPDF_REPLACEMENTS_DESCRIPTION,
    '#default_value' => $field->replacements,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#weight' => 9,
  );

  $form['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/fillpdf/' . $pdf_form->fid,
    '#weight' => 11,
  );

  $form['#pdf_field'] = $field;
  $form['#pdf_form'] = $pdf_form;

  return $form;
}

/**
 * Save changes to the database.
 */
function fillpdf_field_edit_submit($form, &$form_state) {
  if ($form['#pdf_field']) {
    $edit_field = (object) $form_state['values'];
    fillpdf_update_field($form['#pdf_form'], $edit_field, $form['#pdf_field']->pdf_key);
  }
  else {
    // add a new field
    $edit_field = (object) $form_state['values'];
    db_insert('fillpdf_fields')
    ->fields(array(
      'fid' => $form_state['values']['#pdf_form']->fid,
      'label' => $form_state['values']['label'],
      'pdf_key' => $form_state['values']['pdf_key'],
      'value' => empty($form_state['values']['value']) ? '' : $form_state['values']['value'],
      'replacements' => $form_state['values']['replacements'],
    ))
    ->execute();
  }
  $form_state['redirect'] = 'admin/structure/fillpdf/' . $form['#pdf_form']->fid;
}

/**
 * Stores the updated $field in the database
 */
function fillpdf_update_field(&$pdf_form, &$field, $old_key) {
  db_update('fillpdf_fields')
  ->fields(array(
    'label' => $field->label,
    'value' => empty($field->value) ? '' : $field->value,
    'replacements' => $field->replacements,
  ))
  ->condition('fid', $pdf_form->fid)
  ->condition('pdf_key', $old_key)
  ->execute();
}

function _fillpdf_admin_token_form() {
  return array(
    '#theme' => 'token_tree',
    '#token_types' => array('node', 'webform'),
  );
}

