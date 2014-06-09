<?php

/**
 * @file
 * Manages Salesforce webform mapping UI
 */

/**
 * Menu Callback function to show an overview of the existing mappings.
 *
 * Also gives the option to add a new one.
 *
 * @param object $node
 *   The node which owns the webform.
 *
 * @return string
 *   The rendered HTML of the map table.
 */
function salesforce_webforms_manage($node) {
  $maps = salesforce_webforms_get_webform_maps($node);
  $output = '';
  $output .= theme('salesforce_webforms_manage_overview', array('maps' => $maps, 'node' => $node));
  $output .= theme('salesforce_webforms_manage_add_map', array('nid' => $node->nid));
  return $output;
}

/**
 * Get the list of maps associated with the webform.
 *
 * @param object $node
 *   The node which owns the webform.
 *
 * @return array
 *   An array of map arrays.
 */
function salesforce_webforms_get_webform_maps($node) {
  if (in_array($node->type, webform_variable_get('webform_node_types'))) {
    $webform_nid = $node->nid;
    $maps = salesforce_webforms_get_node_maps($node->nid);
  }
  return $maps;
}

/**
 * Returns HTML for a list of object mappings.
 *
 * @param array $variables
 *   An associative array containing:
 *   - maps: The list of mappings for this form.
 *   - node: The node which owns this form.
 *
 * @ingroup themeable
 */
function theme_salesforce_webforms_manage_overview($variables) {
  $maps = $variables['maps'];
  $node = $variables['node'];

  $header = array(t('Map name'), t('Object'), t('Fields'), array(
    'data' => t('Operations'),
    'colspan' => 2,
    ));

  $objects = salesforce_webforms_get_objects_selection();
  if (!empty($maps)) {
    foreach ($maps as $map) {
      $field_info = salesforce_webforms_map_fields_basic($map['sf_object'], $map['fields']);
      $row = array();
      $row[] = array(
        'data' => check_plain($map['mapname']),
      );
      $row[] = array(
        'data' => $objects[$map['sf_object']],
      );
      $row[] = array(
        'data' => theme('item_list', array('items' => $field_info)),
      );
      $row[] = array(
        'data' => l(t('Edit'), 'node/' . $node->nid . '/webform/salesforce/edit/' . $map['sf_object'] . '/' . $map['mapid'], array('query' => drupal_get_destination())),
      );
      $row[] = array(
        'data' => l(t('Delete'), 'node/' . $node->nid . '/webform/salesforce/delete/' . $map['mapid'], array('query' => drupal_get_destination())),
      );
      $rows[] = $row;
    }
  }
  else {
    $rows[][] = array(
      'data' => t('No mappings available.'),
      'colspan' => 5,
    );
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form constructor for the resubmission form.
 *
 * Allows resubmission of webform data to Salesforce.
 *
 * @param object $node
 *   The node which owns this form.
 * @param object $sub
 *   The submission object.
 *
 * @see salesforce_webforms_resubmit_submit()
 *
 * @ingroup forms
 */
function salesforce_webforms_resubmit($form, $form_state, $node, $sub) {
  $form = array();

  $sid = $sub->sid;

  $form["markup"] = array('#markup' => "<h1>" . t('Resubmit webform submission !sid', array('!sid' => $sid)) . "</h1>");
  $form["sid"] = array(
    '#type' => 'hidden',
    '#value' => $sid,
  );

  $form["update"] = array(
    '#type' => 'checkboxes',
    '#options' => array(
      '1' => "",
    ),
    '#title' => t('Update previously created Salesforce objects?'),
  );

  $form["submit"] = array(
    '#type' => 'submit',
    '#value' => t('Send to Salesforce'),
  );

  $form_state["node"] = $node;
  $form_state["submission"] = $sub;

  return $form;
}

/**
 * Form submission handler for salesforce_webforms_resubmit form.
 *
 * @see salesforce_webforms_resubmit()
 */
function salesforce_webforms_resubmit_submit($form, $form_state) {
  $sub = $form_state['submission'];
  $update = $form_state['values']['update']['1'];
  $node = $form_state['node'];

  salesforce_webforms_webform_submission_insert($node, $sub, $update);

  drupal_set_message(t('Salesforce updated'));
}

/**
 * Form constructor for the manage maps form.
 *
 * Allows the form administrator to manage mappings between webform and
 * Salesforce.
 *
 * @param object $node
 *   The node which owns the form.
 * @param string $action
 *   The action to perform. One of:
 *   - add
 *   - edit
 * @param string $obj
 *   The name of the Salesforce object being mapped.
 * @param array $map
 *   The current contents of the mapping.
 *
 * @see salesforce_webforms_manage_map_submit()
 *
 * @ingroup forms
 */
function salesforce_webforms_manage_map($form, $form_state, $node, $action, $obj, $map = NULL) {
  $form = array();
  $obj_info = salesforce_webforms_get_object_fields($obj);

  $form['map'] = array(
    '#type' => 'fieldset',
    '#title' => ($action == 'edit') ? t('Edit mapping') : t('Add mapping'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['map']['object'] = array(
    '#type' => 'hidden',
    '#value' => $obj,
  );

  $form['map']['action'] = array(
    '#type' => 'hidden',
    '#value' => $action,
  );

  if ($action == 'edit' && $map) {
    $form['map']['mapid'] = array(
      '#type' => 'hidden',
      '#value' => $map['mapid'],
    );
  }
  $form['map']['nid'] = array(
    '#type' => 'hidden',
    '#value' => $node->nid,
  );

  $form['map']['mapname'] = array(
    '#type' => 'textfield',
    '#title' => t('Map name'),
    '#default_value' => (isset($map['mapname'])) ? $map['mapname'] : '',
    '#required' => TRUE,
    '#size' => 60,
    '#maxlength' => 64,
    '#weight' => 1,
  );

  $form['map']['map_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Fields'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#weight' => 2,
    '#tree' => TRUE,
  );

  foreach ($obj_info as $fname => $fdata) {
    // Skip if not writable.
    if ($fdata['createable'] == FALSE || $fdata['updateable'] == FALSE) {
      continue;
    }

    $form['map']['map_fields'][$fname] = array(
      '#type' => 'fieldset',
      '#title' => $fdata['label'],
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
    );

    $tmp = array(
      '#title' => t('Source data'),
      '#required' => FALSE,
    );

    if ($fdata['type'] == 'textarea') {
      $tmp['#type'] = 'textarea';
    }
    else {
      $tmp['#maxlength'] = 32767;
      $tmp['#type'] = 'textfield';
    }
    $form['map']['map_fields'][$fname]['target'] = $tmp;

    // Special handling extra select field for some types.
    if ($fdata['type'] == 'picklist') {
      $form['map']['map_fields'][$fname]['picklist'] = array(
        '#type' => 'select',
        '#title' => t('Or select a predefined value'),
        '#options' => salesforce_webforms_picklist_select($fdata['picklistValues']),
      );
    }
    elseif ($fdata['type'] == 'boolean') {
      $form['map']['map_fields'][$fname]['picklist'] = array(
        '#type' => 'select',
        '#title' => t('Or select a predefined value'),
        '#options' => array('' => '', 'false' => 'FALSE', 'true' => 'TRUE'),
      );
    }
    elseif ($fdata['type'] == 'reference') {
      $options = array('' => '');
      // Get a list of all records of the appropriate type(s).
      foreach ($fdata['referenceTo'] as $ref) {
        // Get the object details to see if it supports the IsDeleted attribute.
        $ref_details = salesforce_webforms_get_object_fields($ref);
        $sql = "SELECT id";
        if (isset($ref_details['Name'])) {
          $sql .= ",name";
        }
        // Special handling for Cases.
        if (isset($ref_details['CaseNumber'])) {
          $sql .= ",CaseNumber";
        }
        if (isset($ref_details['DeveloperName'])) {
          $sql .= ",developername";
        }
        $sql .= " from $ref";
        if (isset($ref_details['IsDeleted'])) {
          $sql .= " WHERE IsDeleted=FALSE";
        }
        $targets = _salesforce_webforms_fetch("query?q=" . urlencode($sql));
        foreach ($targets['records'] as $row) {
          $options[$row['Id']] = $ref . ": ";
          if (isset($row['Name'])) {
            $options[$row['Id']] .= $row['Name'];
          }
          elseif (isset($row['CaseNumber'])) {
            $options[$row['Id']] .= $row['CaseNumber'];
          }
          if (isset($row['DeveloperName'])) {
            $options[$row['Id']] .= " (" . $row['DeveloperName'] . ")";
          }
        }
      }
      $form['map']['map_fields'][$fname]['picklist']
        = array(
          '#type' => 'select',
          '#title' => t('Or select a predefined value'),
          '#options' => $options,
        );
    }

    $form['map']['map_fields'][$fname]['keyfield'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Key field?'),
      '#options' => array('1' => ""),
    );
  }

  // Set default values if available.
  if ($map) {
    foreach ($map['fields'] as $field) {
      $form['map']['map_fields'][$field['sf_fieldname']]['#collapsed'] = FALSE;
      $form['map']['map_fields'][$field['sf_fieldname']]['target']['#default_value'] = $field['source_data'];
      if ($form['map']['map_fields'][$field['sf_fieldname']]['picklist']) {
        $form['map']['map_fields'][$field['sf_fieldname']]['picklist']['#default_value'] = array($field['source_data']);
      }
      $form['map']['map_fields'][$field['sf_fieldname']]['keyfield']['#default_value'] = $field['keyfield'] ? array('1') : array();
    }
  }

  $form['map']['submit'] = array(
    '#type' => 'submit',
    '#value' => (isset($map['mapid'])) ? t('Save map') : t('Add map'),
    '#weight' => 25,
  );

  return $form;
}

/**
 * Form validation handler for salesforce_webforms_manage_map().
 *
 * @see salesforce_webforms_manage_map()
 */
function salesforce_webforms_manage_map_validate($form, &$form_state) {
  // Make sure that the map name is unique and formatted properly.
  $name = $form_state['values']['mapname'];
  $mapid = $form_state['values']['mapid'];
  $nid = $form_state['values']['nid'];

  // First make sure that the name is valid.
  if (!preg_match('/^[a-z0-9_]+$/i', $name)) {
    form_set_error('name',
      t('The map name %mapname is invalid. Please include only alphanumeric characters and underscores.',
      array(
        '%mapname' => $name,
      ))
    );

    // No need to process further if the format is invalid.
    return;
  }

  // Now make sure the key isn't duplicated.
  $maps = salesforce_webforms_get_node_maps($nid);
  foreach ($maps as $map) {
    if ($map['mapname'] == $name && $map['mapid'] != $mapid) {
      form_set_error('name',
        t('The map name %mapname is already in use. Please use a unique name.',
        array(
          '%mapname' => $name,
        ))
      );

      // No need to continue processing.
      return;
    }
  }
}

/**
 * Form submission handler for salesforce_webforms_manage_map().
 *
 * @see salesforce_webforms_manage_map()
 */
function salesforce_webforms_manage_map_submit($form, &$form_state) {
  $values = $form_state['values'];
  salesforce_webforms_map_save($values);
}

/**
 * Helper function to convert a Saleforce picklist into an options array.
 *
 * @param array $picklist
 *   An array of data from Salesforce defining the picklist.
 *
 * @return array
 *   An array suitable for use as an 'options' element in a form.
 */
function salesforce_webforms_picklist_select($picklist) {
  $options = array('' => '');

  foreach ($picklist as $pick) {
    if ($pick['active']) {
      $options[$pick['value']] = $pick['label'];
    }
  }

  return $options;
}

/**
 * Gets all webform components on a node as an array keyed by cid.
 *
 * @param object $node
 *   The node which owns the form.
 *
 * @return array
 *   An associative array mapping the component ID to the name.
 */
function salesforce_webforms_get_webform_components($node) {
  $ret = array();
  $components = $node->webform['components'];
  foreach ($components as $cid => $component) {
    $ret[$cid] = $component['name'];
  }

  return $ret;
}

/**
 * Gets all of the picklists available for the named object.
 *
 * @param string $obj
 *   The Salesforce name of the object to be queried.
 *
 * @return array
 *   An associative array of all valid picklists.
 */
function salesforce_webforms_picklists($obj) {
  $data = array();
  $fields = salesforce_webforms_get_object_fields($obj);

  foreach ($fields as $field) {
    // Is this a picklist?
    if ($field['type'] == 'picklist') {
      $data[$field['name']] = array(
        'label' => $field['label'],
        'picklist' => $field['picklistValues'],
      );
      if ($field['dependentPicklist'] == 1) {
        $data['control'] = $field['controllerName'];
      }
    }
  }

  return json_encode($data);
}
