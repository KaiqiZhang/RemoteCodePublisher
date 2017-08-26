function btn(id) {
    if (id > 0) {
        var unfold = document.getElementById(id);
        var fold = document.getElementById(-parseInt(id));

        unfold.style.display = 'none';
        fold.style.display = '';
    }
    else {
        var unfold = document.getElementById(-parseInt(id));
        var fold = document.getElementById(id);

        unfold.style.display = '';
        fold.style.display = 'none';
    }
}