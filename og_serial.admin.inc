<?php

/**
 * @file
 * Admin relate function, page callback.
 */

/**
 * Overview of the serial counter information in group.
 */
function og_serial_overview_page() {
  // og_create_field.
  return drupal_get_form('og_serial_admin_form');
}

/**
 * Form for serial settings.
 */
function og_serial_admin_form($form, &$form_state) {
  $og_context = og_context();
  $group_type = $og_context['group_type'];
  $gid = $og_context['gid'];
  $entity_info = entity_get_info();
  $form['group_content'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );
  foreach (og_get_all_group_content_bundle() as $entity_type => $bundles) {
    $form['group_content'][$entity_type] = array(
      '#type' => 'fieldset',
      '#title' => $entity_info[$entity_type]['label'],
      '#tree' => TRUE,
    );
    $options = array();
    $options_default = array();
    foreach ($bundles as $bundle => $bundle_label) {
       $options[$bundle] = $bundle_label;
       if (og_serial_ogs_is_installed($group_type, $gid, $entity_type, $bundle)) {
         $options_default[] = $bundle;
       }
    }
    $form['group_content'][$entity_type]['bundles'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $options_default,
      '#tree' => TRUE,
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );

  return $form;
}

/**
 * Submit for og_serial_admin_form.
 */
function og_serial_admin_form_submit(&$form, &$form_state) {
  $og_context = og_context();
  $group_type = $og_context['group_type'];
  $gid = $og_context['gid'];
  $group_content = $form_state['values']['group_content'];
  foreach ($group_content as $entity_type => $bundles) {
    foreach ($bundles['bundles'] as $bundle => $status) {
      // Install
      if ($status && !og_serial_ogs_is_installed($group_type, $gid, $entity_type, $bundle)) {
        og_serial_ogs_install($group_type, $gid, $entity_type, $bundle);
      }
      // Uninstall
      if (!$status && og_serial_ogs_is_installed($group_type, $gid, $entity_type, $bundle)) {
        og_serial_ogs_uninstall($group_type, $gid, $entity_type, $bundle);
      }
    }
  }
}
