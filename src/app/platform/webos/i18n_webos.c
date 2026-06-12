#include "util/i18n.h"
#include "util/path.h"

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <webosi18n_C.h>

#include "logging.h"

static ResBundleC *bundle = NULL;
static char locale_[16] = "\0";
static char language[4] = "\0";

/* Convert locale "pt-BR" to path format "pt/BR" to match PackageWebOS directory structure */
static void locale_to_path(const char *locale, char *path, size_t path_size) {
    if (!locale || !path || path_size < 2) {
        return;
    }
    size_t i = 0, j = 0;
    while (locale[i] && j < path_size - 1) {
        path[j++] = (locale[i] == '-') ? '/' : locale[i];
        i++;
    }
    path[j] = '\0';
}

const char *locstr(const char *msgid) {
    if (!bundle) {
        return msgid;
    }
    const char *translated = resBundle_getLocString(bundle, msgid);
    return translated ? translated : msgid;
}

const char *i18n_locale() {
    return locale_;
}

bool i18n_is_loaded(void) {
    return bundle != NULL;
}

void i18n_setlocale(const char *locale) {
    if (bundle) {
        resBundle_destroy(bundle);
        bundle = NULL;
    }
    language[0] = '\0';
    locale_[0] = '\0';

    const char *home = SDL_getenv("HOME");
    if (!home || !locale || !locale[0]) {
        return;
    }

    size_t locale_len = strlen(locale) + 1;
    if (locale_len > sizeof(locale_)) {
        locale_len = sizeof(locale_);
    }
    SDL_memcpy(locale_, locale, locale_len);
    locale_[sizeof(locale_) - 1] = '\0';

    char *resources_root = path_join(home, "resources");
    char locale_path[16];
    locale_to_path(locale, locale_path, sizeof(locale_path));
    bundle = resBundle_createWithRootPath(locale_path, "cstrings.json", resources_root);
    if (!bundle) {
        commons_log_error("I18N", "Failed to load locale '%s' (%s/%s/cstrings.json)", locale, resources_root,
                          locale_path);
        free(resources_root);
        return;
    }

    const char *probe = resBundle_getLocString(bundle, "Cancel");
    if (probe == NULL || strcmp(probe, "Cancel") == 0) {
        commons_log_warn("I18N", "Locale '%s' bundle loaded but strings look untranslated", locale);
    } else {
        commons_log_info("I18N", "Locale '%s' loaded", locale);
    }
    free(resources_root);

    if (SDL_strlen(locale) > 2) {
        SDL_memcpy(language, locale, 2);
        language[2] = '\0';
    }
}

const char *app_get_locale_lang() {
    if (!language[0]) {
        return "C";
    }
    return language;
}
