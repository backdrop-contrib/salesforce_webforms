<?php

/**
 * @file
 * Provides API and management functions for the salesforce webform maps.
 */

/**
 * Get a map entry.
 *
 * @param int $mapid
 *   The ID of the map to load.
 *
 * @return array
 *   An associative array of map data containing:
 *   - mapid: (int) Map ID.
 *   - mapname: (string) The name of the map.
 *   - nid: (int) The node ID that owns the webform.
 *   - sf_object: (string) The Salesforce object being mapped to.
 */
function salesforce_webforms_get_map($mapid) {
  $result = db_query("SELECT mapid, mapname, nid, sf_object FROM {salesforce_webforms_map} where mapid = :mapid", array(":mapid" => $mapid), array('fetch' => PDO::FETCH_ASSOC));
  $map = $result->fetchAssoc();
  $map['fields'] = salesforce_webforms_get_map_fields($mapid, $map['nid']);
  return $map;
}

/**
 * Gets an array of maps assigned to a webform node.
 *
 * @param int $nid
 *   The node ID of the node which owns the webform.
 *
 * @return array
 *   An array of associative arrays containing:
 *   - mapid: (int) Map ID.
 *   - mapname: (string) The name of the map.
 *   - nid: (int) The node ID that owns the webform.
 *   - sf_object: (string) The Salesforce object being mapped to.
 *   - displayorder: (int) The order the mappings should be shown.
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
 *
 * @param int $mapid
 *   The ID of the map to load.
 * @param int $nid
 *   The ID of the node which owns the webform.
 *
 * @return array
 *   An array of associative arrays of fields containing:
 *   - fieldmapid: (int) The ID of this field map.
 *   - sf_fieldname: (string) The Salesforce object being mapped to.
 *   - source_data: (string) The data to be written to Salesforce. May be a
 *     literal string, or may contain tokens from the webform.
 *   - keyfield: (bool) Indicates if this field should be used as a lookup to
 *     determine which object should be updated or not.
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
 *
 * @param int $mapid
 *   The ID of the map to be deleted.
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
 *
 * @param string $obj
 *   The name of the Salesforce object to read.
 * @param array $fields
 *   An associative array of fields as returned from Salesforce.
 *
 * @return array
 *   An associative array, where the keys are the Salesforce field names and
 *   the values are the Salesforce field labels.
 */
function salesforce_webforms_map_fields_basic($obj, array $fields) {
  $ret = array();
  $desc = salesforce_webforms_get_object_fields($obj);
  if ($fields) {
    foreach ($fields as $details) {
      $fname = $details['sf_fieldname'];
      $ret[$fname] = $desc[$fname]['label'];
    }
  }

  return $ret;
}

/**
 * Gets the Salesforce keys assigned to a mapping.
 *
 * @param int $sid
 *   The webform submission ID.
 *
 * @return array
 *   An associative array where the keys are the name of the map, as the
 *   values are the Salesforce IDs of the object created from those maps for
 *   the given submission.
 */
function salesforce_webforms_get_submission_keys($sid) {
  $keys = array();
  $result = db_query("SELECT mapname,sfid FROM {salesforce_webforms_key} WHERE sid = :sid", array(':sid' => $sid));
  foreach ($result as $row) {
    $keys[$row->mapname] = $row->sfid;
  }

  return $keys;
}
