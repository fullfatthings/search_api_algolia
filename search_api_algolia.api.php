<?php


/**
 * @file
 * Hooks provided by the Search API Algolia search module.
 */

/**
 * Lets modules alter the indexed packets sent to algolia.
 *
 * @param array $algolia_data
 *   An array of the default packets, keyed by their Search API ID.
 * @param object &$algolia_index
 *   The Index object instance being used by algolia.
 *
 */
function hook_search_api_algolia_data_alter(&$algolia_data, \AlgoliaSearch\Index $algolia_index) {
  foreach ($items as $node_id => &$item) {
    // Alter items as needed.
  }
}


/**
 * Lets modules alter the IDs sent to algolia for delete operations.
 * Needed as the counterpart of hook_search_api_algolia_data_alter if that changes
 * the IDs sent to algolia for indexing.
 *
 * @param array $ids
 *   The IDs from search_api to be deleted.
 * @param object &$algolia_index
 *   The Index object instance being used by algolia.
 *
 */
function hook_search_api_algolia_ids_alter(&$ids, \AlgoliaSearch\Index $algolia_index) {
  foreach ($ids as &$id) {
    // Change IDs if needed.
  }
}
