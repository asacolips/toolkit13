<?php
/**
 * @file fillpdf.rules.inc
 *
 * Come on, you know what this is for! It's in the name!
 * (But if you don't - it's for Rules integration.)
 */

/**
 * *************
 * *Rules hooks*
 * *************
 */


/**
 * Implements hook_rules_data_info().
 * @todo: Define the fillpdf data structure.
 *   See http://drupal.org/node/905632
 */
function fillpdf_rules_data_info() {
  return array(
    'fillpdf' => array(
      'label' => t('Fill PDF metadata'),
      'group' => t('Fill PDF'),
      'property info' => _fillpdf_rules_metadata_info(),
    ),
  );
}

/**
 * Implements hook_rules_event_info().
 * @todo: Define the "Fill PDF has filled the PDF" event
 * @todo: Define the following events:
 *   - Fill PDF is about to prepare PDF-filling data (fillpdf_merge_pre_merge)
 *   - Fill PDF is ready to fill the PDF (fillpdf_merge_fields_alter)
 */
function fillpdf_rules_event_info() {
  $defaults = array(
    'group' => t('Fill PDF'),
    'module' => 'fillpdf',
  );
  return array(
    'fillpdf_merge_pre_handle' => $defaults + array(
      'label' => t('Filled PDF is about to be handled'),
      'variables' => array(
        'fillpdf' => array('type' => 'fillpdf', 'label' => 'Fill PDF metadata'),
      ),
    )
  );;
}

/**
 * Implements hook_rules_action_info().
 * @todo: Define the following actions:
 *   - Fill a PDF with data from content
 *   - Fill a PDF with data from Webform submissions
 *   - Send PDF to user's browser
 *   - Generate a Fill PDF link (saves new variable - could be useful for e-mail
 *   templates and such)
 */
function fillpdf_rules_action_info() {
  $defaults = array(
    'group' => t('Fill PDF'),
  );
  return array(
    'fillpdf_load' => $defaults + array(
      'label' => t('Load a Fill PDF configuration'),
      'base' => 'fillpdf_rules_action_load_fillpdf',
      'provides' => array(
        'fillpdf' => array(
          'type' => 'fillpdf',
          'label' => t('Fill PDF metadata'),
        ),
      ),
      'parameter' => array(
        'fid' => array(
          'type' => 'integer',
          'label' => t('Fill PDF Form ID'),
        )
      ),
    ),
    'fillpdf_merge_webform' => $defaults + array(
      'label' => t('Fill a PDF with webform data'),
      'base' => 'fillpdf_rules_action_merge_webform',
      'description' => t('Populates the PDF with webform data and updates the
        Rules variable with all information necessary to handle it.'),
      'parameter' => array(
        'fillpdf' => array(
          'type' => 'fillpdf',
          'label' => t('Fill PDF metadata'),
        ),
        'webform_nid' => array(
          'type' => 'integer',
          'label' => t('Webform Node ID'),
          'optional' => TRUE,
          'description' => t('If you leave this blank, the <em>Default Node ID</em> from the Fill PDF configuration will be used.'),
        ),
        'webform_sids' => array(
          'type' => 'list<integer>',
          'label' => t('Webform Submission ID(s)'),
          'optional' => TRUE,
          'description' => t('If you leave this blank, the most recent submission will be used. The last ID you specify will be checked first.'),
        ),
      ),
    ),
    'fillpdf_handle_default' => $defaults + array(
      'label' => t('Perform the default action on the PDF'),
      'description' => t('Handle the PDF according to its Fill PDF configuration.'),
      'base' => 'fillpdf_rules_action_handle_default',
      'parameter' => array(
        'fillpdf' => array(
          'type' => 'fillpdf',
          'label' => t('Fill PDF metadata'),
        )
      ),
    ),
    'fillpdf_save_to_file' => $defaults + array(
      'label' => t('Save PDF to a file'),
      'base' => 'fillpdf_rules_action_save_to_file',
      'provides' => array(
        'fillpdf_saved_file_path' => array(
          'type' => 'text',
          'label' => t('Path to saved PDF'),
        ),
      ),
      'parameter' => array(
        'fillpdf' => array(
          'type' => 'fillpdf',
          'label' => t('Fill PDF metadata'),
        )
      ),
    ),
    'fillpdf_delete_saved_file' => $defaults + array(
      'label' => t('Delete saved PDF'),
      'base' => 'fillpdf_rules_action_delete_file',
      'parameter' => array(
        'filename' => array(
          'type' => 'text',
          'label' => t('Filename of PDF to delete'),
        ),
      ),
    )
  );
}

/**
 * Implements hook_rules_condition_info().
 * @todo: Define the following conditions:
 *   - A node is being filled
 *   - A Webform is being filled
 */
function fillpdf_rules_condition_info() {

}

/**
 * *****************
 * *Rules callbacks*
 * *****************
 */

// Action callbacks //

function fillpdf_rules_action_load_fillpdf($fid) {
  $fillpdf = new stdClass;
  $fillpdf->info = fillpdf_load($fid);
  return array(
    'fillpdf' => $fillpdf,
  );
}

/**
 * Populates a loaded Fill PDF configuration's PDF with data.
 */
function fillpdf_rules_action_merge_webform($fillpdf, $webform_nid = '', $webform_sids = array()) {
  $skip_access_check = FALSE;
  $flatten = TRUE;
  $webforms = array();
  foreach ($webform_sids as $sid) {
    $webforms[] = array('nid' => $webform_nid,
      'sid' => $sid,
    );
  }

  if (empty($webforms) && empty($webform_nid) !== TRUE) {
    $webforms[0]['nid'] = $webform_nid;
  }

  // @todo: Parameterize $skip_access_check and $flatten in Rules.
  $fillpdf = fillpdf_merge_pdf($fillpdf->info->fid, NULL, $webforms, NULL, FALSE, $skip_access_check, $flatten, FALSE);
  return array('fillpdf' => $fillpdf);
}

/**
 * Save the PDF to a file and return the file path.
 */
function fillpdf_rules_action_save_to_file($fillpdf) {
  $saved_file_path = fillpdf_save_to_file($fillpdf->info, $fillpdf->data, $fillpdf->token_objects, _fillpdf_process_filename($fillpdf->info->title, $fillpdf->token_objects), FALSE);
  return array(
    'fillpdf_saved_file_path' => $saved_file_path,
  );
}

/**
 * Perform the default action on the PDF. This always ends in a drupal_goto() or a drupal_exit().
 */
function fillpdf_rules_action_handle_default($fillpdf) {
  fillpdf_merge_handle_pdf($fillpdf->info, $fillpdf->data, $fillpdf->token_objects, 'default');
}

function fillpdf_rules_action_delete_file($filename) {
  file_unmanaged_delete($filename);
}

// Condition callbacks //

// Metadata callbacks //
function _fillpdf_rules_metadata_info() {
  return array();
}

