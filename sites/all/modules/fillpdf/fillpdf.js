(function ($) {
  Drupal.behaviors.fillpdf = {
    attach: function (context, settings) {
      $('#edit-fillpdf-service input').change(function () {
        if (this.value === 'remote') {
          $('#edit-remote').removeClass('collapsed');
        }
        else {
          $('#edit-remote').addClass('collapsed');
        }
      });
    }
  };
}(jQuery));
