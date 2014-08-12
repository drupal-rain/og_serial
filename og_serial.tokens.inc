<?php

/**
 * @file
 * Token function for og_serial.
 */

/**
 * Implements hook_token_info().
 */
function og_serial_token_info() {
  // @todo Support other than 'node'.
  $info = array();
  $info['types']['og_serial'] = array(
    'name' => t('OG Serial'),
    'description' => t('Tokens related to og_serial.'),
    'needs-data' => 'node',
  );
  $info['tokens']['og_serial']['ogs_id'] = array(
    'name' => t('ogs_id'),
    'description' => t('og_serial table id.'),
  );
  $info['tokens']['og_serial']['sid'] = array(
    'name' => t('sid'),
    'description' => t('Serial number.'),
  );

  return $info;
}

/**
 * Implements hook_token_info_alter().
 */
function og_serial_token_info_alter(&$info) {
  $fields = field_info_fields();
  foreach ($info['tokens']['node'] as $name => &$token_info) {
    if (isset($fields[$name]) && $fields[$name]['type'] == 'og_serial') {
      $token_info['type'] = 'og_serial';
    }
  }
}

/**
 * Implements hook_tokens().
 */
function og_serial_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  // @todo Support other than 'node'.
  if ($type == 'node' && !empty($data['node'])
    && !empty($data['node']->og_serial)
  ) {
    $node = $data['node'];
    $language = entity_language('node', $node);
    if (!isset($node->nid)) {
      $og_context = og_context();
      $group_type = $og_context['group_type'];
      $gid = $og_context['gid'];
      $ogs_id = og_serial_ogs_get_id($group_type, $gid, 'node', $node->type);
      $sid = og_serial_ogs_get_counter($ogs_id) + 1;
    }
    else {
      $ogs_id = $node->og_serial[$language][0]['ogs_id'];
      $sid = $node->og_serial[$language][0]['sid'];
    }
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'og_serial':
          // @todo Better information?
          $replacements[$original] = $sid;
          break;
        case 'og_serial:ogs_id':
          $replacements[$original] = $ogs_id;
          break;
        case 'og_serial:sid':
          $replacements[$original] = $sid;
          break;
      }
    }
  }

  return $replacements;
}
