<?php
/**
 * @file
 * Returns the HTML for a node.
 *
 * Complete documentation for this file is available online.
 * @see https://drupal.org/node/1728164
 */
?>
<article class="node-<?php print $node->nid; ?> <?php print $classes; ?> clearfix"<?php print $attributes; ?>>

  <?php if ($title_prefix || $title_suffix || $display_submitted || $unpublished || !$page && $title): ?>
    <header>
      <?php print render($title_prefix); ?>
      <?php if (!$page && $title): ?>
        <h2<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h2>
      <?php endif; ?>
      <?php print render($title_suffix); ?>

      <?php if ($display_submitted): ?>
        <p class="submitted">
          <?php print $user_picture; ?>
          <?php print $submitted; ?>
        </p>
      <?php endif; ?>

      <?php if ($unpublished): ?>
        <mark class="unpublished"><?php print t('Unpublished'); ?></mark>
      <?php endif; ?>
    </header>
  <?php endif; ?>

  <?php
    // We hide the comments and links now so that we can render them later.
    hide($content['comments']);
    hide($content['links']);
    print render($content);


    //build a new menu using a specific menu and parent mlid    
    $config = array(
      'delta' => 0,
      'menu_name' => 'menu-editor-menu',
      'parent_mlid' => 965,
      'title_link' => FALSE,
      'level' => 1,
      'follow' => 0,
      'depth' => 1,
      'expanded' => FALSE,
      'sort' => FALSE
      );

    $menu = menu_tree_build($config);
    $menu_links = $menu['content']['#content'];
    $options = array();

    ctools_include('jump-menu');

    //get the value and title from the current menu links
    foreach ($menu_links as $menu_link) {
      if ( empty($menu_link['#title'])) {
        continue;
      }

      $options[url($menu_link['#href'])] = $menu_link['#title'];
    }
    //build and render the form
    $form = drupal_get_form('ctools_jump_menu', $options);
    print render($form);
  ?>

  <?php print render($content['links']); ?>

  <?php print render($content['comments']); ?>

</article>
