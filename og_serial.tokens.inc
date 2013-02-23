<?php

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
  if ($type == 'node' && !empty($data['node'])) {
    $node = $data['node'];
    $language = entity_language('node', $node);
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'og_serial':
          // @todo Better information?
          $replacements[$original] = $node->og_serial[$language][0]['sid'];
          break;
        case 'og_serial:ogs_id':
          $replacements[$original] = $node->og_serial[$language][0]['ogs_id'];
          break;
        case 'og_serial:sid':
          $replacements[$original] = $node->og_serial[$language][0]['sid'];
          break;
      }
    }
  }

  return $replacements;
}
