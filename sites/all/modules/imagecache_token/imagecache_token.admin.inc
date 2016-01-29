<?php
/**
 * @file
 * Settings form for the Imagecache Token module.
 */

/**
 * FormAPI callback to load the imagecache_token settings form.
 */
function imagecache_token_settings_form() {
  $fields = field_info_fields();

  $options = array();
  foreach ($fields as $field) {
    if ($field['type'] == 'file' || $field['type'] == 'media') {
      foreach ($field['bundles'] as $entity_type => $bundles) {
        foreach ($bundles as $bundle) {
          $key = $field['type'] . ':' . $field['field_name'] . ':' . $entity_type . ':' . $bundle;
          $label = $entity_type . ': ' . $bundle . ': ' . $field['field_name'];
          $options[$key] = $label;
        }
      }
    }
  }

  $form['imagecache_token_fields'] = array(
    '#title' => t('"File" and "Media" fields that contain images'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get('imagecache_token_fields', array()),
  );

  if (!empty($options)) {
    $form['imagecache_token_fields']['#description'] = t('All "image" fields are automatically given tokens for each image style currently on the system. "File" and "Media" fields <em>may</em> be used for images, but as they may also be used for other types of files the fields that <em>are</em> used for images must be indicated.');
  }
  else {
    $form['imagecache_token_fields']['#description'] = t('There are no "file" or "media" fields, so there is nothing to configure.');
  }

  return system_settings_form($form);
}

/**
 * FormAPI submission callback for imagecache_token_settings_form.
 */
function imagecache_token_settings_form_submit($form, &$form_state) {
  // Clear the token caches.
  cache_clear_all('*', 'cache_token', TRUE);
  drupal_static_reset('token_info');

  // Reload the token info.
  token_info();
}
