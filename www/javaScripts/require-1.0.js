/// require

// region header

/**
    @author t.sickert@gmail.com (Torben Sickert)
    @version 1.0 stable
    @fileOverview
    It provides also a global require function for defining JavaScript
    module dependencies.

    Conventions (LCx := library convention number x)

    - lc1 Capitalized variables are constant and shouldn't be mutable.
    - lc2 Properties with preceding underscores shouldn't be accessed from
          the outer scope. They could accessed in inherited objects
          (protected attributes).
    - lc3 Property with two preceding underscore shouldn't be accessed from
          any location then the object itself (private attributes).
    - lc4 Follow the javascript OOP conventions like camel-case class-names
          methods and property names.
    - lc5 Class-names have a leading upper case letter.
    - lc6 Methods and functions are starting with a lower case letter.
    - lc7 Do not use more chars then 79 in one line.
    - lc8 Use short and/or long description doc-strings for all definitions.
    - lc9 Write qunit tests for each unit it is possible and try to reach 100%
          path coverage.
    - lc10 Sorting imports as following:
               1. Import all standard modules and packages,
               2. then all from third party,
               3. now import your own modules or packages.
               4. Sort import names alphabetically and seperate the previous
                  defined parts with blank lines.
    - lc11 Import everthing by its whole name and reference path and use it by
           its full reference path (even builtin units).
    - lc12 Don't use any abbreviations.
    - lc13 Try to use small cyclomatice complexity in all units.
           (e.g. less than 20 or 30).
    - lc14 Use one of the plugin pattern described in "jQuery.Tools".
    - lc15 Use the area statement syntax to structure your code and make it
           possible to fold them in many IDE's
           (see Structure of meta documenting below).
    - lc16 Always think that code is more read than written.
    - lc17 By choosing witch quotes to use follow this priority.
               1. Single quote (')
               2. Double quote (")
    - lc18 Indent function parameter which doesn't match in one line like:

           function(
               parameter1, parameter2, parameter3,
               parameter4)

           instead of:

           function(parameter1,
                    parameter2,
                    parameter3,
                    parameter4)

Structure of meta documenting classes. (see lc15)

    // region header

    ;window.require([['ia', 'ia-1.0'], ['ib', 'ib-2.0']]), function() {

    // endregion

    // region plugins

    (function(ia) {

        var A = function() {

        // region (Public|Protected) (properties|methods)

            // region Property of method or property group

                // region Subproperty of method or property group

            ...

                // endregion

            // endregion

        // endregion

        };

    // endregion

    // region footer

    });

    // endregion

Structure of dependencies

    0. window
    1. window.require
    3. jQuery
    4. jQuery.Tools
    5. jQuery.*

    This means that a module in level "i" could only import a full module
    in its header in level "j" if "j < i" is valid.
*/


/** @name window */
;

// endregion

// region objects

(function(context) {
    /**
        This class can be used as function for defining dependencies for
        modules.
        Note that this function searches in the same ressource as the first
        javascript include tag in your markup if given dependency resource
        doesn't start with "http://".
        You can manually change this behavior by adding a search base via
        "window.require.basePath".
        @memberOf window
        @class
        @param {Array[String[]]} modules A list of string array which describes
                                         needed modules. Every element is a
                                         tuple consisting of an object
                                         reference which has to be available
                                         after script was loading and the
                                         module name (basename of script file
                                         without ".js" extension).
        @param {Function} onLoaded A callback function to load after all
                                   dependences are available.
        @param {Object} onLoadedArguments A various number of arguments given
                                          to the "onLoaded" callback function.
        @example
window.require([['jQuery', 'jquery-1.8.2']], function() {
    jQuery('div#id').show('slow');
});
    */
    var require = function(modules, onLoaded, onLoadedArguments) {
        /*
            These properties could be understand as static (or class instead of
            object) properties.
        */

    // region public properties

        /**
            If setted all ressources will be appended by a timestamp string to
            make each request unique.
            This is usefull to workaround some browsers caching mechanisms
            which aren't required.

            @property {Boolean}
        */
        require.appendTimeStamp
        /**
            Indicates if debugging is active.

            @property {Boolean}
        */
        require.logging;
        /**
            Saves the base path for relative defined module locations.

            @property {String}
        */
        require.basePath;
        /**
            If the require scope should be deleted after serving all
            dependencies this property should be an array with a callback
            function and its arguments following. The function will be called
            after last dependency was solved. Simply define true is also
            valid.

            @property {Boolean}
        */
        require.noConflict;
        /**
            Caches a reference to the head for injecting needed script tags.

            @property {DomNode}
        */
        require.scriptNode;
        /**
            Saves all loaded script ressources to prevent double script
            loading.

            @property {String[]}
        */
        require.initializedLoadings;
        /**
            Indicates if require should load ressource on its own.

            @property {Boolean}
        */
        require.passiv;
        /**
            Saves the initially pointed target of global variable
            "window.require" to reset that reference in "noConflict" mode.

            @property {Mixed}
        */
        require.referenceSafe;

    // endregion

    // region protected properties

        /**
            Saves function calls to require for running them in right order to
            guarantee dependencies. It consits of a list of tuples storing
            needed dependency as string and arguments to be given to callback
            function if dependency is determined.

            @property {Object[]}
        */
        require._callQueue;

    // endregion

    // region public methods

        /**
            @description This method is used as initializer. Class properties
                         will be initialized if its the first call to require.
                         This methods gets the same arguments as the global
                         "require" constructor.

            @returns {require} Returns the current instance.
        */
        require.initialize = function() {
            if (require.basePath === undefined) {
                var firstScriptSource = document.getElementsByTagName(
                    'script')[0].src;
                require.basePath = firstScriptSource.substring(
                    0, firstScriptSource.lastIndexOf('/') + 1);
            }
            if (require.appendTimeStamp === undefined)
                require.appendTimeStamp = false;
            if (require.passiv === undefined)
                require.passiv = false;
            if (require.logging === undefined)
                require.logging = false;
            if (require.noConflict === undefined)
                require.noConflict = false;
            if (require.initializedLoadings === undefined)
                require.initializedLoadings = new Array();
            if (require.scriptNode === undefined)
                require.scriptNode = document.getElementsByTagName('head')[0];
            if (require.basePath &&
                require.basePath.substring(require.basePath.length - 1) != '/')
                require.basePath += '/';

            if (require._callQueue === undefined)
                require._callQueue = new Array();
            return require._load.apply(require, arguments);
        };

    // endregion

    // region protected methods

        /**
            @description Loads needed modules and run the "onLoaded" callback
                         function. This methods gets the same arguments as the
                         global "require" constructor.

            @returns {require} Returns the current instance.
        */
        require._load = function() {
            /*
                Convert arguments object to an array.

                This following outcomment line would be responsible for a bug
                in yuicompressor.
                Because of declaration of arguments the parser things that
                arguments is a local variable and could be renamed.
                It doesn't care about that the magic arguments object is
                neccessary to generate the arguments array in this context.

                var arguments = this.argumentsObjectToArray(arguments);
            */
            var parameter = Array.prototype.slice.call(arguments);
            /*
                Make sure that we have a copy of given array containing needed
                dependencies.
            */
            if (parameter[parameter.length - 1] !== require) {
                // Save a copy if initially given dependencies.
                parameter.push(parameter[0].slice(0));
                // Mark array as initialized.
                parameter.push(require);
            }
            /*
                This method is alway working with arguments array for easy
                recursive calling itself with a dynamic number of arguments.
            */
            if (parameter[0].length) {
                // Grab first needed dependency from given queue.
                var module = parameter[0].shift();
                if (typeof(module) === 'object' &&
                    require._isModuleLoaded(module))
                    /*
                        If module is already there make a recursive call with
                        one module dependency less.
                    */
                    require._load.apply(require, parameter);
                /*
                    If module is currently loading put current function call
                    status in queue to continue later.
                */
                else if (
                    typeof(module) === 'string' ||
                    !require._isLoadingInitialized(module[0], parameter)
                ) {
                    if (typeof(module) === 'string')
                        module = ['', module];
                    if (require.passiv)
                        this._log(
                            'Prevent loading module "' + module[0] +
                            '" in passiv mode.');
                    else
                        require._appendScriptNode(
                            require._createScriptLoadingNode(module[1]),
                            module, parameter);
                }
            } else {
                /*
                    Call a given event handler (if provided as second argument)
                    when all dependencies are determined.
                */
                if (parameter.length > 3)
                    parameter[1].apply(
                        context, require._generateLoadedHandlerArguments(
                            parameter));
                /*
                    If other dependencies aren't determined yet try to
                    determine it now after a new dependency was loaded.
                */
                if (require._callQueue.length && require._isModuleLoaded(
                    require._callQueue[require._callQueue.length - 1]))
                    require._load.apply(require, require._callQueue.pop()[1]);
            }
            if (require && require._handleNoConflict)
                return require._handleNoConflict();
            return undefined;
        };
        /**
            @description Generates an array of arguments from initially given
                         arguments to the require constructor. The generated
                         arguments are designed to give loaded handler a useful
                         scope

            @param {Object[]} parameters Initially given arguments.

            @returns {Object[]} Returns an array of arguments.
        */
        require._generateLoadedHandlerArguments = function(parameters) {
            var additionalArguments = new Array();
            /*
                Note: don't use the "for ... in ..." statement, because we
                don't want to iterate over the properties of an array type.
            */
            for (var index = 0;
                 index < parameters[parameters.length - 2].length; index++) {
                if (parameters[parameters.length - 2][index].length === 2) {
                    var moduleObjects =
                        parameters[parameters.length - 2][index][0].split('.');
                    var query = context;
                    /*
                        Note: don't use the "for ... in ..." statement, because
                        we don't want to iterate over the properties of an
                        array type.
                    */
                    for (var subIndex = 0; subIndex < moduleObjects.length;
                         subIndex++) {
                        query = query[moduleObjects[subIndex]];
                        additionalArguments.push(query);
                    }
                }
            }
            return parameters.slice(2, parameters.length - 2).concat(
                additionalArguments, parameters[parameters.length - 2]);
        };
        /**
            @description Appends a given script loading tag inside the dom
                         tree.

            @param {DomNode} ScriptNode Dom node where to append script
                             loading node.
            @param {String[]} module A tuple (consisting of module indicator
                                     and module file path) which should be
                                     loaded.
            @param {Object[]} parameters Saves arguments indented to be given
                                         to the onload function.

            @returns {require} Returns the current instance.
        */
        require._appendScriptNode = function(scriptNode, module, parameters) {
            /*
                Internet explorer workaround for capturing event when
                script is loaded.
            */
            if (scriptNode.readyState)
                scriptNode.onreadystatechange = function() {
                    if (scriptNode.readyState === 'loaded' ||
                        scriptNode.readyState === 'complete') {
                        require._scriptLoaded(module, parameters);
                        // Delete event after passing it once.
                        scriptNode.onreadystatechange = null;
                    }
                };
            else {
                scriptNode.onload = function() {
                    require._scriptLoaded(module, parameters);
                    // Delete event after passing it once.
                    scriptNode.onload = null;
                };
            }
            require.scriptNode.appendChild(scriptNode);
            return require;
        };
        /**
            @description Creates a new script loading tag.

            @param {String} scriptFilePath Path pointing to the file ressource.

            @returns {DomNode} Returns script node needed to load given script
                               ressource.
        */
        require._createScriptLoadingNode = function(scriptFilePath) {
            require._log(
                'Initialize script loading of "' + scriptFilePath + '".');
            var scriptNode = document.createElement('script');
            scriptNode.src = require.basePath + scriptFilePath;
            if (scriptFilePath.substring(0, 7) === 'http://')
                scriptNode.src = scriptFilePath;
            if (scriptNode.src.substr(-3) != '.js')
                scriptNode.src += '.js';
            if (require.appendTimeStamp)
                scriptNode.src += '?timestamp=' + (new Date).getTime();
            scriptNode.type = 'text/javascript';
            return scriptNode;
        };
        /**
            @description If script was loaded it will be deleted from the
                         "initializedLoading" array. If all dependencies for
                         this module are available the sequence could continue
                         oterwise the current sequence status
                         (the parameter array) will be saved in a queue for
                         continue later.

            @param {String[]} module A tuple of module name to indicate if a
                              module is presence and its file path ressource.
            @param {Object[]} parameters Saves arguments indented to be given
                                         to the onload function.

            @returns {require} Returns the current instance.
        */
        require._scriptLoaded = function(module, parameters) {
            for (var key in require.initializedLoadings)
                if (module[0] === require.initializedLoadings[key]) {
                    require.initializedLoadings.splice(key, 1);
                    break;
                }
            if (require._isModuleLoaded(module))
                require._load.apply(require, parameters);
            else
                require._callQueue.push([module[0], parameters]);
            return require;
        };
        /**
            @description If "noConflict" property is set it will be handled
                         by this method. It clear the called scope from the
                         "require" name and optionally runs a callback
                         function given by the "noConflict" property after all
                         dependencies are solved.

            @returns {require} Returns the current instance.
        */
        require._handleNoConflict = function() {
            if (require._callQueue.length === 0 &&
                require.initializedLoadings.length === 0) {
                require._log('All ressources are loaded so far.');
                if (require && require.noConflict)
                    if (require.noConflict === true)
                        /*
                            Restore previous setted value to the "require"
                            reference.
                        */
                        require = require.referenceSafe;
                    else {
                        // Copy not only the reference.
                        var callback = require.noConflict.slice();
                        require = undefined;
                        callback[0].apply(context, callback.slice(1));
                    }
            }
            return require;
        };
        /**
            @description Determines if the given moduleObject is currently
                         loading. If the given module is currently loading
                         the current sequence status will be stored in the
                         "callQueue" for continuing later.

            @param {String} moduleName A module object to indicate if a module
                                       is presence.
            @param {Object[]} parameters The current status of solving the
                                         initially described arguments.

            @returns {Boolean} If given module object is currently loading
                               "true" will be given back and "false" otherwise.
        */
        require._isLoadingInitialized = function(moduleName, parameters) {
            for (var key in require.initializedLoadings)
                if (moduleName === require.initializedLoadings[key]) {
                    require._callQueue.push([moduleName, parameters]);
                    return true;
                }
            if (moduleName)
                require.initializedLoadings.push(moduleName);
            return false;
        };
        /**
            @description Determines if the given moduleObject is present in the
                         global (window) scope.

            @param {String[]} module A tuple of module name to indicate if a
                                     module is presence and its file path.

            @returns {Boolean} If given module object is present this method
                               will return "true" and "false" otherwise.
        */
        require._isModuleLoaded = function(module) {
            var query = context;
            if (module[0]) {
                var moduleObjects = module[0].split('.');
                /*
                    Note: don't use the "for ... in ..." statement, because we
                    don't want to iterate over the properties of an array type.
                */
                for (var index = 0; index < moduleObjects.length; index++)
                    if (query[moduleObjects[index]])
                        query = query[moduleObjects[index]];
                    else {
                        require._log(
                            '"' + module[0] + '" isn\'t available because "' +
                            moduleObjects[index] + '" is missing in "' +
                            query.toString() + '".');
                        return false;
                    }
                require._log('"' + module[0] + '" is loaded complete.');
            } else
                require._log('"' + module[1] + '" is loaded complete.');
            return true;
        };
        /**
            @description If logging is enabled. Method shows the given message
                         in the browsers console if possible or in a standalone
                         alert-window as fallback.

            @param {String} message A logging message.

            @returns {undefined|false} Returns the return value of
                                       "window.console.log()" or
                                       "window.alert()" or "false" if logging
                                       is disabled.
        */
        require._log = function(message) {
            if (require.logging) {
                if (window.console.log)
                    return window.console.log('require: ' + message);
                return window.alert('require: ' + message);
            }
            return false;
        };
        // Run initializer with all given arguments to the require function.
        require.initialize.apply(require, arguments);
    };

    // endregion

    /** @ignore */
    if (require.referenceSafe === undefined)
        require.referenceSafe = context.require;
    context.require = require;
})(window);

// endregion
