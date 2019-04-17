ImageCache Token
----------------
This module provides additional tokens for image fields.

For each image style available, a token [node:field_image_field:STYLENAME] will
be provided.

Various properties for each image style are also available:
* [node:field_image_field:STYLENAME:height] - The image's height.
* [node:field_image_field:STYLENAME:width] - The image's width.
* [node:field_image_field:STYLENAME:mimetype] - The image's MIME type.
* [node:field_image_field:STYLENAME:extension] - The image's file extension.
* [node:field_image_field:STYLENAME:filesize] - The image's file size.
* [node:field_image_field:STYLENAME:path] - The image's path.
* [node:field_image_field:STYLENAME:render] - Full HTML for the image, as
  generated via theme('image_formatter').


Configuration
--------------------------------------------------------------------------------
Because "file" and "media" file types may contain other types of files, not just
images, it is necessary to indicate which fields hold images. Field selection is
controlled via the settings page:
  * admin/config/media/imagecache-token

All image fields are automatically supported by Imagecache Token, nothing needs to be done.

Both "File" and "Media" fields may be used for images, but they may also be
used for other types of files. Select the fields that are used for images so
that tokens can be added for them.


Known Issues
--------------------------------------------------------------------------------
After creating new image style it might not appear immediately in available
tokens list. To resolve this try reseting the cache on the Performance page or
using Drush.

A known problem with the Token module can result in a timeout attempting to load
the token browser if there are a lot of entity types and/or fields on the site.
The problem is most notable when using the Entity Token module part of the
Entity API module, which is required when using the Commerce system. There is no
current fix for the problem, though there are some workarounds available:
* Even if the token browser is unresponsive, the tokens themselves will still
  work, so experiment with manually creating the tokens.
* Use the Token Tweaks module to limit how many levels of the token tree will
  be displayed: https://www.drupal.org/project/token_tweaks
* Collaborate on fixing the problem in the Token module:
  https://www.drupal.org/node/1334456
* Use the Fast Token Browser module which extends the Token module with a faster
  and more usable interface for browsing and inserting Tokens. It resolves
  excessive server-side memory usage and client-side unresponsiveness when
  using the token browser on sites with many tokens. Due to the magnitude of
  memory savings, this allows the token browser to work with a very low PHP
  memory limit, even without needing to constrain the maximum token depth.
  https://www.drupal.org/project/fast_token_browser


Drupal 8 port
--------------------------------------------------------------------------------
Thanks to work from Sascha Grossenbacher [1], Francis de Lasalle [2],
silkogelman [3] and others, the functionality has been merged into the Token module itself, so a separate module will not be needed for D8.


Credits
--------------------------------------------------------------------------------
Currently maintained by Damien McKenna [4]. Originally written by Pavel A.
Karoukin [5] with contributions by Pascal Crott [6] and others in the community.

Ongoing development is sponsored by Mediacurrent [7].

The best way to contact the authors is to submit an issue, be it a support
request, a feature request or a bug report, in the project issue queue:
  https://www.drupal.org/project/issues/metatag


References
--------------------------------------------------------------------------------
1: https://www.drupal.org/u/berdir
2: https://www.drupal.org/u/bambell
3: https://www.drupal.org/u/silkogelman
4: https://www.drupal.org/u/damienmckenna
5: https://www.drupal.org/u/hydra
6: https://www.drupal.org/u/monstrfolk
7: https://www.mediacurrent.com/
