
/**
 * JavaScript behaviors for the front-end display of Salesforce-driven picklists.
 */

(function ($) {

// From https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/keys
// Make sure we have the "keys" function on Object
if (!Object.keys) {
  Object.keys = (function () {
    'use strict';
    var hasOwnProperty = Object.prototype.hasOwnProperty,
        hasDontEnumBug = !({toString: null}).propertyIsEnumerable('toString'),
        dontEnums = [
          'toString',
          'toLocaleString',
          'valueOf',
          'hasOwnProperty',
          'isPrototypeOf',
          'propertyIsEnumerable',
          'constructor'
        ],
        dontEnumsLength = dontEnums.length;

    return function (obj) {
      if (typeof obj !== 'object' && (typeof obj !== 'function' || obj === null)) {
        throw new TypeError('Object.keys called on non-object');
      }

      var result = [], prop, i;

      for (prop in obj) {
        if (hasOwnProperty.call(obj, prop)) {
          result.push(prop);
        }
      }

      if (hasDontEnumBug) {
        for (i = 0; i < dontEnumsLength; i++) {
          if (hasOwnProperty.call(obj, dontEnums[i])) {
            result.push(dontEnums[i]);
          }
        }
      }
      return result;
    };
  }());
}

Drupal.behaviors.salesforce_webforms = Drupal.behaviors.salesforce_webforms || {};

Drupal.behaviors.salesforce_webforms.attach = function(context, settings) {
  // Dependent picklist logic.
  if (Drupal.settings.salesforce_webforms.salesforceMap) {
    Drupal.salesforce_webforms.salesforce(context, Drupal.settings.salesforce_webforms);
  }
};

Drupal.salesforce_webforms = Drupal.salesforce_webforms || {};

Drupal.salesforce_webforms.salesforce = function(context, settings) {
  // Add the bindings to each webform on the page.
  $.each(settings.salesforceMap, function(formId, settings) {
    var $form = $('#' + formId + ':not(.webform-salesforce-processed)');
    if ($form.length) {
      $form.addClass('webform-salesforce-processed');

      $form.bind('change', { 'settings': settings }, Drupal.salesforce_webforms.salesforceCheck);

      // See if we have any dependent fields
      //$.each(settings, function(elementId) {
      for(var elementId in settings) {
        var children = new Array();
        var parent = null;
        var fieldname = settings[elementId].fieldname;
        var controlField = settings[elementId].control;
        for(var child in settings) {
          if(settings[child].control == fieldname) {
            children[children.length] = child;
          }

          if(settings[child].fieldname == controlField) {
            parent = child;
          }
        }

        settings[elementId].parent = parent;
        settings[elementId].children = children;
      }

      // Trigger all the elements that are driven by Salesforce picklists on this form which have children, but no parents
      $.each(settings, function(elementId) {
        if(settings[elementId].parent == null && settings[elementId].children.length > 0) {
          $('#' + elementId).find('select').filter(':first').trigger('change');
        }
      });
    }
  });
};

/**
 * Event handler to respond to field changes in a form.
 *
 * This event is bound to the entire form, not individual fields.
 */
Drupal.salesforce_webforms.salesforceCheck = function(e) {
  var $trigger = $(e.target);
  var triggerId = $trigger.attr('id');
// alert("Trigger ID: " + triggerId);
  var $div = $(e.target).parents('.sf-picklist-wrapper:first');
  // var $form = $trigger.closest('form');
// alert("div: "+JSON.stringify($div));
  var formId = $div.attr('id');
// alert("Form ID: " + formId);
  var settings = e.data.settings;
// alert("Settings: " + JSON.stringify(settings));
// alert("Settings["+formId+"]="+JSON.stringify(settings[formId]));

  for (var i = 0; i < settings[formId].children.length; i++) {
    Drupal.salesforce_webforms.show_hide(triggerId, settings[formId].children[i], settings);
  }
};

Drupal.salesforce_webforms.show_hide = function(tid, idx, settings){
  // Get current value
  var $cmp = $('#' + idx);
  var $sel = $cmp.find("select");

  var curval = $sel.val();
// alert("Saving old value "+curval);

  var options = Drupal.salesforce_webforms.salesforceGetPickList(tid, idx, settings);
  var showComponent;

  if(Object.keys(options).length == 0) {
    // Hide this component
    showComponent = false;
    curval = "";
  }
  else {
    // Show this component
    showComponent = true;
  }
  if (showComponent) {
    var $opt = $cmp.find('select');
// alert("$opt: "+JSON.stringify($opt));
// alert("Options: "+JSON.stringify(options));
    // Get the first option to add back in
    var emptyOpt = $opt.find("option").first();
    $opt.find("option").remove();
    $opt.append(emptyOpt);

    $.each(options, function (key, lbl) {
// alert("Adding option "+key);
      // $opt.append(new Option(key, lbl));
      $opt.append($('<option>', { lbl : key })
          .text(lbl));
                        // .attr("value",key)
                        // .text(lbl));
    });
    $cmp.find('select').removeAttr('disabled').removeClass('salesforce-webform-disabled').end().show();
    $cmp.find('select').focus();
  }
  else {
    // $cmp.find('input:not(:disabled)').attr('disabled', true).val('').addClass('salesforce-webform-disabled').end().hide();
    $cmp.find('select').attr('disabled', true).val(null).addClass('salesforce-webform-disabled').end().hide();
  }

  // Select the old value if possible
  $sel.val(curval);

  // And process any children goes here
  var selid = $sel.attr('id');
  var children = settings[idx].children;
  for(var i = 0; i < children.length; i++) {
    Drupal.salesforce_webforms.show_hide(selid, children[i], settings);
  }
}


Drupal.salesforce_webforms.salesforceGetPickList = function(pid, element, map) {
  var options = new Object();
  var controlFieldName = map[element].control;

  // First, figure out what DOM ID houses this field
  var controlFieldId = null;
  for(fieldId in map) {
    if(map[fieldId].fieldname == controlFieldName)
      controlFieldId = fieldId;
  }

  // Did we find a match?
  if(controlFieldId == null) {
    // Nope
// alert("Didn't find controlling field "+controlFieldName+" for "+element);
    return map[element].options;
  }

  var controlIndex = Drupal.salesforce_webforms.salesforceGetSelectedIndex(controlFieldId, map);
// alert("Got control index "+controlIndex);

  // And determine which of our options apply to that position
  var optionmap = new Array();
  optionmap[0] = -1;
  controlIndex--;
  for (var i = 0; controlIndex >= 0 && i < map[element]['full'].length; i++) {
    if(map[element].full[i].map[controlIndex]) {
      // options[options.length] = new Option(
        // map[element]['full'][i]['label'],
        // map[element]['full'][i]['value']
// alert("Adding option "+map[element].full[i].value);
      options[map[element].full[i].value] = map[element].full[i].label;
// alert("Resulting in "+JSON.stringify(options));
      // Store the map
      optionmap[optionmap.length] = map[element]['full'][i].position;
      // map[element]['full'][i]['displayPosition'] = options.length;
    }
    else {
      map[element]['full'][i]['displayPosition'] = -1;
    }
  }

  map[element].options = options;
  map[element].optionmap = optionmap;

// alert("Returning options list "+JSON.stringify(options));
  return options;
}

Drupal.salesforce_webforms.salesforceGetSelectedIndex = function(el, map) {
  // First, recusively make sure all parents are set to valid values
  // Drupal.salesforce_webforms.show_hide(el, map);

  // And now get the index
  var $elem = $('#' + el);
  var $find = $elem.find('select');
  var idx = $find.attr('selectedIndex');

  // Now map that back to the original order
  var ret = idx == 0 ? -1 : map[el].optionmap[idx];
// alert("Mapping "+el+" from "+idx+" to "+ret);
  return ret;
}
})(jQuery);
