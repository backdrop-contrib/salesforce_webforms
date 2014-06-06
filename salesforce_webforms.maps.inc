<?php

/**
 * @file
 * provides API and management functions for the salesforce webform maps
 */

/**
 * Get a map entry.
 */
function salesforce_webforms_get_map($mapid) {
  $result = db_query("SELECT mapid, mapname, nid, sf_object FROM {salesforce_webforms_map} where mapid = :mapid", array(":mapid" => $mapid), array('fetch' => PDO::FETCH_ASSOC));
  $map = $result->fetchAssoc();
  $map['fields'] = salesforce_webforms_get_map_fields($mapid, $map['nid']);
  return $map;
}

/**
 * Get an array of maps assigned to a webform node.
 */
function salesforce_webforms_get_node_maps($nid) {
  $maps = array();
  $result = db_query("SELECT mapid, mapname, nid, sf_object, displayorder FROM {salesforce_webforms_map} WHERE nid = :nid ORDER BY displayorder", array(':nid' => $nid), array('fetch' => PDO::FETCH_ASSOC));
  foreach ($result as $map) {
    $map['fields'] = salesforce_webforms_get_map_fields($map['mapid'], $map['nid']);
    $maps[] = $map;
  }
  return $maps;
}

/**
 * Get an array of fields mapped in a map.
 */
function salesforce_webforms_get_map_fields($mapid, $nid) {
  $fids = array();
  $fields = array();
  $result = db_query("SELECT fieldmapid,sf_fieldname,source_data,is_keyfield FROM {salesforce_webforms_field} WHERE mapid = :mapid", array(':mapid' => $mapid));
  foreach ($result as $row) {
    $fields[] = array(
      'fieldmapid' => $row->fieldmapid,
      'sf_fieldname' => $row->sf_fieldname,
      'source_data' => $row->source_data,
      'keyfield' => $row->is_keyfield,
    );
  }

  return $fields;
}

/**
 * Deletes a mapping and dependencies.
 */
function salesforce_webforms_delete_mapping($mapid) {
  // Delete mapping.
  db_delete('salesforce_webforms_map')
    ->condition('mapid', $mapid)
    ->execute();

  // Delete fields.
  db_delete('salesforce_webforms_field')
    ->condition('mapid', $mapid)
    ->execute();
}

/**
 * Helper function.
 *
 * Takes the full list of fields and returns a basic
 * representation for display purposes.
 */
function salesforce_webforms_map_fields_basic($obj, $fields) {
  $ret = array();
  $desc = salesforce_webforms_get_object_info($obj);
  if ($fields) {
    foreach ($fields as $details) {
      $fname = $details['sf_fieldname'];
      $ret[$fname] = $desc[$fname];
    }
  }

  return $ret;
}

/**
 * Gets the Salesforce keys assigned to a mapping.
 */
function salesforce_webforms_get_submission_keys($sid) {
  $keys = array();
  $result = db_query("SELECT mapname,sfid FROM {salesforce_webforms_key} WHERE sid = :sid", array(':sid' => $sid));
  foreach ($result as $row) {
    $keys[$row->mapname] = $row->sfid;
  }

  return $keys;
}
