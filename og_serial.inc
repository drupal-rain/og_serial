<?php

/**
 * og_serial include file.
 */

/**
 * Install a new counter for a group content.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_ogs_install($group_type, $gid, $entity_type, $bundle) {
  $data = array(
    'group_type' => $group_type,
    'gid' => $gid,
    'entity_type' => $entity_type,
    'bundle' => $bundle,
    'counter' => 0,
  );
  drupal_write_record('og_serial', $data);
  // Create the field as well.
  og_create_field(OG_SERIAL_FIELD, $entity_type, $bundle);
  // Initiate for the existing group content.
  og_serial_gc_initiate($group_type, $gid, $entity_type, $bundle);
}

/**
 * Uninstall a counter for a group content.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_ogs_uninstall($group_type, $gid, $entity_type, $bundle) {
  db_delete('og_serial')
    ->condition('group_type', $group_type)
    ->condition('gid', $gid)
    ->condition('entity_type', $entity_type)
    ->condition('bundle', $bundle)
    ->execute();
  // @todo Only delete the field if no implementation of this group content bundle.
  // Use cron for garbage collection?
  // Deinitiate for the existing group content.
  og_serial_gc_deinitiate($group_type, $gid, $entity_type, $bundle);
}

/**
 * Check if the counter is installed.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_ogs_is_installed($group_type, $gid, $entity_type, $bundle) {
  if (og_serial_ogs_get_id($group_type, $gid, $entity_type, $bundle) != FALSE) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Get the current counter number.
 *
 * Get counter in two ways: 1) id; 2) mandatory.
 */
function og_serial_ogs_get_counter($id, $group_type = NULL, $gid = NULL, $entity_type = NULL, $bundle = NULL) {
  $query = db_select('og_serial', 'ogs')
    ->fields('ogs', array('counter'));
  if (isset($id)) {
    $query->condition('id', $id);
  }
  else {
    $query->condition('group_type', $group_type)
      ->condition('gid', $gid)
      ->condition('entity_type', $entity_type)
      ->condition('bundle', $bundle);
  }
  $result = $query
    ->execute()
    ->fetchField();
  
  return $result;
}

/**
 * Forward the counter one step.
 */
function og_serial_ogs_increase_counter($id, $group_type = NULL, $gid = NULL, $entity_type = NULL, $bundle = NULL) {
  $query = db_update('og_serial')
    ->expression('counter', 'counter + :increment', array(':increment' => 1));
  if (isset($id)) {
    $query->condition('id', $id);
  }
  else {
    $query->condition('group_type', $group_type)
      ->condition('gid', $gid)
      ->condition('entity_type', $entity_type)
      ->condition('bundle', $bundle);
  }
  $result = $query->execute();
}

function og_serial_ogs_reset_counter($id, $group_type = NULL, $gid = NULL, $entity_type = NULL, $bundle = NULL) {
  
}

/**
 * Get the og_serial unique id.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_ogs_get_id($group_type, $gid, $entity_type, $bundle) {
  $result = db_select('og_serial', 'ogs')
    ->fields('ogs', array('id'))
    ->condition('group_type', $group_type)
    ->condition('gid', $gid)
    ->condition('entity_type', $entity_type)
    ->condition('bundle', $bundle)
    ->execute()
    ->fetchField();

  return $result;
}

/**
 * Return the group content list.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 *
 * @return array $group_contents Group content list.
 */
function og_serial_gc_list($membership_type, $group_type, $gid, $entity_type, $bundle) {
  $conditions = array(
    'type' => $membership_type,
    'group_type' => $group_type,
    'gid' => $gid,
    'entity_type' => $entity_type,
  );
  $og_memberships = og_membership_load_multiple(FALSE, $conditions);
  $group_contents = array();
  foreach ($og_memberships as $og_membership) {
    $entity = entity_load_single($entity_type, $og_membership->etid);
    list($id, $vid, $type) = entity_extract_ids($entity_type, $entity);
    if ($type == $bundle) {
      $group_contents[] = $entity;
    }
  }

  return $group_contents;
}

/**
 * Initiate the serial number for group content.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_gc_initiate($group_type, $gid, $entity_type, $bundle) {
  // 1. Find all the group content of the group.
  // @todo Support other membership type?
  $group_contents = og_serial_gc_list(OG_MEMBERSHIP_TYPE_DEFAULT, $group_type, $gid, $entity_type, $bundle);
  $ogs_id = og_serial_ogs_get_id($group_type, $gid, $entity_type, $bundle);
  // 2. Insert the serial number to og_serial field.
  foreach ($group_contents as $group_content) {
    $group_content->{OG_SERIAL_FIELD}[$group_content->language][0] = array(
      'ogs_id' => $ogs_id,
      'sid' => og_serial_ogs_get_counter($ogs_id) + 1,
    );
    entity_save($entity_type, $group_content);
    og_serial_ogs_increase_counter($ogs_id);
  }
}

/**
 * Deinitiate the serial number for group content.
 *
 * @param string $group_type
 * @param int $gid
 * @param string $entity_type
 * @param string $bundle
 */
function og_serial_gc_deinitiate($group_type, $gid, $entity_type, $bundle) {
  $group_contents = og_serial_gc_list(OG_MEMBERSHIP_TYPE_DEFAULT, $group_type, $gid, $entity_type, $bundle);
  $ogs_id = og_serial_ogs_get_id($group_type, $gid, $entity_type, $bundle);
  foreach ($group_contents as $group_content) {
    $group_content->{OG_SERIAL_FIELD}[$group_content->language] = array();
    entity_save($entity_type, $group_content);
  }
}

/**
 * Theme function for the default formatter.
 */
function theme_og_serial_formatter_default($variables) {
  return $variables['sid'];
}
