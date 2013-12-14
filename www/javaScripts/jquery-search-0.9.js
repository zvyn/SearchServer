;window.require(
    ['less-1.3.0',

     ['jQuery.Tools', 'jquery-tools-1.0'],

     ['jQuery.effects', 'jquery-ui-1.8.24/jquery.effects.core'],
     ['jQuery.effects.fade', 'jquery-ui-1.8.24/jquery.effects.fade'],

     ['jQuery.ui', 'jquery-ui-1.8.24/jquery.ui.core'],
     ['jQuery.widget', 'jquery-ui-1.8.24/jquery.ui.widget'],
     ['jQuery.ui.autocomplete', 'jquery-ui-1.8.24/jquery.ui.autocomplete'],

     ['jQuery.fn.ajaxForm', 'jquery-form-2.69'],
     ['jQuery.cookie', 'jquery-cookie-1.3']],
    function(jQuery
) {
    var Search = function(domNode) {
        this._options = {
            'domNodeSelectorPrefix': 'body.{1}',
            'autocompleteRequestUrl': '',
            'logging': false,
            'domNodes': {
                'searchInput': 'form input[type="text"][name="searchQuery"]',
                'submitButton': 'form input[type="submit"]',
                'searchForm': 'form',
                'results': 'div.results',
                'numberOfResultsInput':
                    'form input[type="text"][name="number"]',
                'numberOfSuggestionsInput':
                    'form input[type="text"][name="numberOfSuggestions"]'
            }
        };
        this._domNodes = {};
        this._lastMatches = [];
        this._numberOfResultsDefault = 4;
        this._numberOfSuggestionsDefault = 4;

        this.initialize = function(options) {
            this._options.domNodeSelectorPrefix = this.stringFormat(
                this._options.domNodeSelectorPrefix,
                this.camelCaseStringToDelimited(this.__name__));
            if (options)
                jQuery.extend(true, this._options, options);
            // Merge default plugin-options with given optons.
            if (options)
                jQuery.extend(true, this._options, options);
            // Grap all needed dom nodes.
            this._domNodes = this.grapDomNodes(this._options.domNodes);
            this._numberOfResultsDefault =
                this._domNodes.numberOfResultsInput.attr('value');
            this._numberOfSuggestionsDefault =
                this._domNodes.numberOfSuggestionsInput.attr('value');
            var self = this;
            this._ajaxSearchOptions = {
                'dataType': 'jsonp',
                'success': self.getMethod(self._handleSearchRequest),
                'jsonpCallback': 'searchRecordsCallback',
                'cache': true,
                'beforeSubmit': function(formData) {
                    jQuery.each(formData, function(key, value) {
                        value.value = jQuery.trim(value.value);
                    });
                }
            };
            // Initialize autocompletion
            this._domNodes.searchInput.autocomplete({
                'source': this.getMethod(this._processQuery),
                'select': function(event) {
                    if (event.toElement)
                        jQuery(this).attr('value', event.toElement.innerText);
                        self._domNodes.searchForm.ajaxSubmit(
                            self._ajaxSearchOptions);
                }
            });
            this.bind(
                this._domNodes.numberOfResultsInput, 'change', function() {
                    if (!jQuery(this).attr('value'))
                        self._domNodes.numberOfResultsInput.attr(
                            'value', self._numberOfResultsDefault);
                });
            this.bind(
                this._domNodes.numberOfSuggestionsInput, 'change', function() {
                    if (!jQuery(this).attr('value'))
                        self._domNodes.numberOfSuggestionsInput.attr(
                            'value', self._numberOfSuggestionsDefault);
                });
            this.bind(this._domNodes.searchInput, 'keyup', function() {
                self._domNodes.searchForm.ajaxSubmit(self._ajaxSearchOptions);
            });
            this._domNodes.searchForm.ajaxForm(this._ajaxSearchOptions);
            this._preventOtherInputThanNumber(
                this._domNodes.numberOfResultsInput.add(
                    this._domNodes.numberOfSuggestionsInput));
            this._setNumbersFromCookie(
                this._domNodes.numberOfResultsInput.add(
                    this._domNodes.numberOfSuggestionsInput));
            this._setNumbersToCookie(
                this._domNodes.numberOfResultsInput.add(
                    this._domNodes.numberOfSuggestionsInput));
            this._domNodes.submitButton.fadeOut(3000);
            return this;
        };

        this._setNumbersToCookie = function(domNodes) {
            this.bind(domNodes, 'keyup', function(event) {
                var date = new Date();
                var minutes = 1;
                date.setTime(date.getTime() + (minutes * 60 * 1000));
                jQuery.cookie(
                    jQuery(this).attr('name'), jQuery(this).attr('value'),
                    {'expires': date});
            });
            return this;
        };

        this._setNumbersFromCookie = function(domNodes) {
            domNodes.each(function() {
                if (jQuery.cookie(jQuery(this).attr('name')))
                    jQuery(this).attr('value', jQuery.cookie(
                        jQuery(this).attr('name')));
            });
            return this;
        };

        this._preventOtherInputThanNumber = function(domNodes) {
            this.bind(domNodes, 'keydown', function(event) {
                if (jQuery.inArray(
                        event.keyCode,
                        [jQuery.ui.keyCode.BACKSPACE,
                         jQuery.ui.keyCode.DELETE,
                         jQuery.ui.keyCode.LEFT,
                         jQuery.ui.keyCode.RIGHT]) == -1 &&
                    (event.keyCode < 48 || event.keyCode > 57) &&
                    (event.keyCode < 96 || event.keyCode > 105))
                    event.preventDefault();
            });
            return this;
        };

        this._processQuery = function(query, setMatchesCallbackFunction) {
            jQuery.ajax({
                'url': this._options.autocompleteRequestUrl,
                'data': {
                    'vocabularyLookup': jQuery.trim(query.term),
                    'number':
                        this._domNodes.numberOfSuggestionsInput.attr('value')
                },
                'dataType': 'jsonp',
                'success': function(data) {
                    setMatchesCallbackFunction(data.matches);
                },
                'cache': true,
                'jsonpCallback': 'similarWordsCallback'});
            return this;
        };

        this._handleSearchRequest = function(data) {
            if (!this._areMatchesEqual(data.matches)) {
                var self = this;
                this._domNodes.results.fadeOut('fast', function() {
                    self._domNodes.results.html('');
                    if (data.matches.length) {
                        jQuery.each(data.matches, function(key, value) {
                            self._domNodes.results.append(jQuery('<a>').attr(
                                'href', value
                            ).text(value.substring(
                                value.lastIndexOf('/') + 1)));
                        });
                    } else
                        self._domNodes.results.append(jQuery('<div>').text(
                            'No results found.'));
                    self._domNodes.results.fadeIn('fast');
                });
            }
            this._lastMatches = data.matches;
            return this;
        };

        this._areMatchesEqual = function(matches) {
            if (matches.length !== this._lastMatches.length)
                return false;
            result = true;
            var self = this;
            jQuery.each(matches, function(key, value) {
                if (jQuery.inArray(value, self._lastMatches) === -1) {
                    result = false;
                    return false;
                }
            });
            return result;
        };
    };

    jQuery.fn.Search = function() {
        var self = jQuery.Tools()._extend(new Search(this));
        self.__name__ = 'Search';
        return self._controller.apply(self, arguments);
    };
});
