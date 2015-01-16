<?php
/**
 * Implements hook_default_rules_configuration().
 */
function fillpdf_default_rules_configuration() {
  $default_rules = array();
  // Attach the filled PDF to an e-mail.
  if (module_exists('mimemail') && module_exists('webform_rules') && module_exists('webform')) {
    $default_rules['rules_webform_fillpdf_attachment'] = rules_import('{ "rules_send_webform_submission_confirmation_with_filled_pdf_as_attachment" : {
        "LABEL" : "Send Webform submission confirmation e-mail with filled PDF as attachment",
        "PLUGIN" : "reaction rule",
        "ACTIVE" : false,
        "REQUIRES" : [ "rules", "fillpdf", "mimemail", "webform_rules" ],
        "ON" : [ "webform_rules_submit" ],
        "IF" : [ { "data_is" : { "data" : [ "node:nid" ], "value" : "2" } } ],
        "DO" : [
          { "fillpdf_load" : {
              "USING" : { "fid" : "6" },
              "PROVIDE" : { "fillpdf" : { "fillpdf" : "Fill PDF metadata" } }
            }
          },
          { "fillpdf_merge_webform" : {
              "fillpdf" : [ "fillpdf" ],
              "webform_nid" : "2",
              "webform_sids" : { "value" : [] }
            }
          },
          { "fillpdf_save_to_file" : {
              "USING" : { "fillpdf" : [ "fillpdf" ] },
              "PROVIDE" : { "fillpdf_saved_file_path" : { "fillpdf_saved_file_path" : "Path to saved PDF" } }
            }
          },
          { "mimemail" : {
              "to" : [ "site:mail" ],
              "subject" : "Copy of completed PDF for your records",
              "body" : "Thank you for filling out the form on our website. A copy of your completed PDF is attached.",
              "plaintext" : " Thank you for filling out the form on our website. A copy of your completed PDF is attached.",
              "attachments" : [ "fillpdf-saved-file-path" ]
            }
          },
          { "fillpdf_delete_saved_file" : { "filename" : [ "fillpdf-saved-file-path" ] } }
        ]
      }
    }');
  }
  return $default_rules;
}

