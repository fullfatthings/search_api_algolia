<?php

/**
 * @file
 * Contains SearchApiAlgoliaService.
 */

/**
 * Indexes and searches items using Algolia.
 */
class SearchApiAlgoliaService extends SearchApiAbstractService implements SearchApiServiceInterface {


  /**
   * Implements SearchApiServiceInterface::__construct().
   *
   * The sets $this->server and $this->options.
   * Also initiates the Algolia client as $this->client if the options for YourApplicationID and YourAPIKey
   * have been set.
   */
  public function __construct(SearchApiServer $server) {
    libraries_load('algolia_api');
    $this->server = $server;
    $this->options = &$server->options;

    if ($this->isConfigured()) {
      $this->client = new \AlgoliaSearch\Client($this->options['application_id'], $this->options['api_key']);
    }
  }


  /**
   * Do we have the properties to instantiate the API client?
   * @return boolean
   */
  protected function isConfigured() {
    return isset($this->options['application_id']) && isset($this->options['api_key']);
  }


  /**
   * Form constructor for the server configuration form.
   *
   * Might be called with an incomplete server (no ID). In this case, the form
   * is displayed for the initial creation of the server.
   *
   * @param array $form
   *   The server options part of the form.
   * @param array $form_state
   *   The current form state.
   *
   * @return array
   *   A form array for setting service-specific options.
   */
  public function configurationForm(array $form, array &$form_state) {
    $form['application_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Algolia Application ID'),
      '#size' => 20,
      '#maxlength' => 20,
      '#required' => TRUE,
    );

    $form['api_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Algolia API Key'),
      '#size' => 40,
      '#maxlength' => 40,
      '#required' => TRUE,
    );

    $form['search_only_api_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Algolia Search only API Key'),
      '#size' => 40,
      '#maxlength' => 40,
      '#required' => TRUE,
    );

    // Set default values in form if they exist.
    foreach ($form as $key => $value) {
      if (isset($this->options[$key])) {
        $form[$key]['#default_value'] = $this->options[$key];
      }
    }

    return $form;
  }

  /**
   * Indexes the specified items.
   *
   * @param SearchApiIndex $index
   *   The search index for which items should be indexed.
   * @param array $items
   *   An array of items to be indexed, keyed by their id. The values are
   *   associative arrays of the fields to be stored, where each field is an
   *   array with the following keys:
   *   - type: One of the data types recognized by the Search API, or the
   *     special type "tokens" for fulltext fields.
   *   - original_type: The original type of the property, as defined by the
   *     datasource controller for the index's item type.
   *   - value: The value to index.
   *
   *   The special field "search_api_language" contains the item's language and
   *   should always be indexed.
   *
   *   The value of fields with the "tokens" type is an array of tokens. Each
   *   token is an array containing the following keys:
   *   - value: The word that the token represents.
   *   - score: A score for the importance of that word.
   *
   * @return array
   *   An array of the ids of all items that were successfully indexed.
   *
   * @throws SearchApiException
   *   If indexing was prevented by a fundamental configuration error.
   */
  public function indexItems(SearchApiIndex $index, array $items) {
    // Store original keys before any hooks alter them.
    $original_keys = array_keys($items);

    // Note the max size of a record for Algolia is 10KB.
    $algolia_data = array();

    foreach ($items as $id => $item) {
      $algolia_data[$id] = array('objectID' => $id);

      foreach ($item as $key => $field) {
        $algolia_data[$id][$key] = $field['value'];
      }
    }

    // Initiate index (not an expensive operation - does not make any API calls).
    $algolia_index = $this->client->initIndex($index->machine_name);

    // Let other modules alter the data that is indexed.
    drupal_alter('search_api_algolia_data', $algolia_data, $algolia_index);

    // Send data to Algolia.
    try {
      $algolia_index->addObjects(array_values($algolia_data));
    }
    catch (AlgoliaException $e) {
      throw new SearchApiException($e->getMessage());
    }

    return $original_keys;
  }

  /**
   * Deletes indexed items from this server.
   *
   * Might be either used to delete some items (given by their ids) from a
   * specified index, or all items from that index, or all items from all
   * indexes on this server.
   *
   * @param $ids
   *   Either an array containing the ids of the items that should be deleted,
   *   or 'all' if all items should be deleted. Other formats might be
   *   recognized by implementing classes, but these are not standardized.
   * @param SearchApiIndex $index
   *   The index from which items should be deleted, or NULL if all indexes on
   *   this server should be cleared (then, $ids has to be 'all').
   *
   * @throws SearchApiException
   *   If an error occurred while trying to delete the items.
   */
  public function deleteItems($ids = 'all', SearchApiIndex $index = NULL) {
    $algolia_index = $this->client->initIndex($index->machine_name);

    try {
      if ($ids == 'all') {
        $algolia_index->clearIndex();
      }
      else {
        // Let other modules alter the ids of the data that is indexed.
        drupal_alter('search_api_algolia_ids', $ids, $algolia_index);
        $algolia_index->deleteObjects($ids);
      }
    }
    catch (AlgoliaException $e) {
      throw new SearchApiException($e->getMessage());
    }
  }

  /**
   * Executes a search on the server represented by this object.
   *
   * @param $query
   *   The SearchApiQueryInterface object to execute.
   *
   * @return array
   *   An associative array containing the search results, as required by
   *   SearchApiQueryInterface::execute().
   *
   * @throws SearchApiException
   *   If an error prevented the search from completing.
   */
  public function search(SearchApiQueryInterface $query) {
    $count = 0;

    $index_name = $query->getIndex()->machine_name;

    $indexes = $this->client->listIndexes();

    foreach ($indexes['items'] as $index) {
      if ($index['name'] == $index_name) {
        $count = $index['entries'];
      }
    }

    // Todo - return real results.
    return array(
      'result count' => $count,
      'results' => array()
    );
  }
}
