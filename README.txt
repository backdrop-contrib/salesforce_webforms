Salesforce Suite Webform Integration README

ABOUT
=============================
This module is designed to allow an easy integration between the Salesforce
Suite module and the Webform module. Data can be written to Salesforce
objects, and Salesforce picklist fields can be used on the forms presented
to end-users.

INSTALLATION AND CONFIGURATION
==============================
Install and Enable Webform module
Install and Enable Salesforce Suite module
Configure a remote application within Salesforce
Authenticate Salesforce Suite by navigating to

Admin -> Configuration -> Salesforce -> Authenticate

and entering the Customer Access Token and Customer Secret Key

USAGE
==============================

  On the form presented to users:
    A new form control of type 'Salesforce Picklist' should be available.
    Choosing this should present you with a list of all picklists defined for
    all objects in your Salesforce intance. Please note that populating this
    list the first time can take a significant amount of time. Once run, the
    data is stored in a cache, so subsequent uses should load much faster.

    Salesforce picklists are presented to the user in the form of a dropdown.
    The data can be presented in the order defined in Salesforce, sorted
    alphabetically, or in a random order. If some options defined within
    Salesforce should be excluded from the list when displayed to the user,
    those items can be specified in the appropriate field, one item per
    line. The value specified for the exclusion should be the value of the
    item, and not the label. If translation is not enabled in Salesforce,
    then those two items will be the same. However, if a given picklist
    entry IS translated, then the label will be the translated version,
    while the value will be the untranslated version. The untranslated
    value should be used for exlusion.

    If a picklist is defined in Salesforce to be dependent on another list,
    and that list is also included on the same form, then the dependent
    picklist will automatically be updated to reflect the currently valid
    values based on the controlling list. If no values are valid for the
    currently selected controlling value, then the dependent list will be
    hidden.

    If translation is enabed for the picklist, then the values presented to
    the user will be in the language of the calling page, if available. If
    no translation is available, then the base language label will be used.
    In all cases, the value returned by the control will be the base language
    version.

  To send data to Salesforce:
    Data entered by the user, as well as hidden values and hard coded
    information can be sent to Salesforce to create and/or update
    Salesforce objects. Multiple objects can be created from a single form,
    in a cascading fashion. For instance, a contact record can be created
    based on information provided by the user, and then that contact
    record can be used to populate the Contact field of a Case record
    create from other (or the same) data on that form. There are no enforced
    limits on the number or type of objects created, though, as more objects
    are added, performance will decrease, as each involves a round-trip
    connection between your Drupal instance and Salesfore.

    These outgoing Salesforce connections, called mappings, are defined
    in a manner similar to creating an email message from webforms. Each
    mapping is associated with a single Saleforce object, and must have a name
    which is used to store the ID of the Salesforce object created or updated.
    Within that object, each available field will be shown, and a valude can
    be provided. If no value is provided, then that field will be left
    empty.

    Any fields with the 'Key' attribute set will be used to attempt to
    update an existing record. If no key fields are set, or no records
    exist which match all of the key fields, then a new record will be
    created. If a record can be found which matches all key fields, then
    it will be update, and it's ID associated with that mapping.

    The possible values for a given field vary based on the type of the field.
    Text fields provide for a simply single line value. Textarea fields allow
    for multi-line values. Picklists, boolean, and referenced object values
    allow for a text value, or a dropdown list of currently defined values.

    To use a value provided by the user, the syntax would be

      [submission:values:KEY] or [submission:values:KEY:nolabel]
      (Webform 4.x)

    or

      %value[key]
      (Webform 3.x)

    where KEY is the name of the field in the webform, and :nolabel indicates
    that the field label should NOT be included in the data.

    Multiple values can be combined into a single field. For instance, if
    you collect first name in a field called 'fname' and last name in a field
    called 'lname' but you want to update a field called 'Full Name' in
    Salesforce, the value might be

      [submission:values:fname:nolabel] [submission:values:lname:nolabel]
      (Webform 4.x)

    or

      %value[fname] %value[lname]
      (Webform 3.x)

    To include the ID of an object created in an earlier mapping, the syntax
    would be

      [submission:salesforce:MAPNAME]
      (All versions of webform)

    For instance, if a Contact mapping were created with the name 'newcontact'
    and you want that contact associated with a case that's also being
    created, then, in the case mapping, you would set the Contac ID to be

      [submission:salesforce:newcontact]

    Once a form is submitted, all of the data will be sent immediately to
    Salesforce. If the data should need to be sent again, view the submission
    data within webforms. There will be a link on the data allowing the option
    to resend the data to Salesforce. When sending the data, the administrator
    will have the option of updating the Salesforce objects created earlier,
    or to create new objects.

TROUBLESHOOTING
===========================================
Error: refresh_token is blank
  This error happens after the Salesforce session times out. The Salesforce
  Suite module can automatically refresh the session, but only if the
  application is granted the proper permission inside Salesforce. See the
  Salesforce Suite documentation for details.
