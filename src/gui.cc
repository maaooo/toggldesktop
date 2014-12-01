
// Copyright 2014 Toggl Desktop developers.

#include "./gui.h"

#include <cstdlib>
#include <sstream>

#include "Poco/Stopwatch.h"

namespace toggl {

void GUI::DisplayApp() {
    on_display_app_(true);
}

void GUI::DisplayLogin(const _Bool open, const uint64_t user_id) {
    std::stringstream ss;
    ss << "DisplayLogin open=" << open << ", user_id=" << user_id;
    logger().debug(ss.str());

    on_display_login_(open, user_id);
}

_Bool GUI::DisplayError(const error err) {
    if (noError == err) {
        return true;
    }

    logger().error(err);

    if (isNetworkingError(err)) {
        std::stringstream ss;
        ss << "You are offline (" << err << ")";
        DisplayOnlineState(false, ss.str());
        return false;
    }

    logger().debug("DisplayError");

    char_t *err_s = copy_string(err);
    on_display_error_(err_s, isUserError(err));
    free(err_s);

    return false;
}

error GUI::VerifyCallbacks() {
    logger().debug("VerifyCallbacks");
    error err = findMissingCallbacks();
    if (err != noError) {
        logger().error(err);
    }
    return err;
}

error GUI::findMissingCallbacks() {
    if (!on_display_app_) {
        return error("!on_display_app_");
    }
    if (!on_display_error_) {
        return error("!on_display_error_");
    }
    if (!on_display_online_state_) {
        return error("!on_display_online_state_");
    }
    if (!on_display_login_) {
        return error("!on_display_login_");
    }
    if (!on_display_url_) {
        return error("!on_display_url_");
    }
    if (!on_display_reminder_) {
        return error("!on_display_reminder_");
    }
    if (!on_display_time_entry_list_) {
        return error("!on_display_time_entry_list_");
    }
    if (!on_display_time_entry_autocomplete_) {
        return error("!on_display_time_entry_autocomplete_");
    }
    if (!on_display_project_autocomplete_) {
        return error("!on_display_project_autocomplete_");
    }
    if (!on_display_workspace_select_) {
        return error("!on_display_workspace_select_");
    }
    if (!on_display_client_select_) {
        return error("!on_display_client_select_");
    }
    if (!on_display_tags_) {
        return error("!on_display_tags_");
    }
    if (!on_display_time_entry_editor_) {
        return error("!on_display_time_entry_editor_");
    }
    if (!on_display_settings_) {
        return error("!on_display_settings_");
    }
    if (!on_display_timer_state_) {
        return error("!on_display_timer_state_");
    }
    if (!on_display_idle_notification_) {
        return error("!on_display_idle_notification_");
    }
    return noError;
}

void GUI::DisplayReminder() {
    logger().debug("DisplayReminder");

    char_t *s1 = copy_string("Reminder from Toggl Desktop");
    char_t *s2 = copy_string("Don't forget to track your time!");
    on_display_reminder_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayOnlineState(const _Bool is_online, const std::string reason) {
    logger().debug("DisplayOnlineState");

    char_t *reason_s = copy_string(reason);
    on_display_online_state_(is_online, reason_s);
    free(reason_s);
}

void GUI::DisplayUpdate(const _Bool open,
                        const std::string update_channel,
                        const _Bool is_checking,
                        const _Bool is_available,
                        const std::string url,
                        const std::string version) {
    if (!on_display_update_) {
        logger().debug("This GUI has own update system, will not use lib.");
        return;
    }

    logger().debug("DisplayUpdate");

    TogglUpdateView view;
    view.UpdateChannel = copy_string(update_channel);
    view.IsChecking = is_checking;
    view.IsUpdateAvailable = is_available;
    view.URL = copy_string(url);
    view.Version = copy_string(version);

    on_display_update_(open, &view);

    free(view.UpdateChannel);
    free(view.URL);
    free(view.Version);
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<toggl::AutocompleteItem> *items) {
    logger().debug("DisplayTimeEntryAutocomplete");

    TogglAutocompleteView *first = 0;
    for (std::vector<toggl::AutocompleteItem>::const_reverse_iterator it =
        items->rbegin(); it != items->rend(); it++) {
        TogglAutocompleteView *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    on_display_time_entry_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::AutocompleteItem> *items) {
    logger().debug("DisplayProjectAutocomplete");

    TogglAutocompleteView *first = 0;
    for (std::vector<toggl::AutocompleteItem>::const_reverse_iterator it =
        items->rbegin(); it != items->rend(); it++) {
        TogglAutocompleteView *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    on_display_project_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayTimeEntryList(const _Bool open,
                               TogglTimeEntryView* first) {
    Poco::Stopwatch stopwatch;
    stopwatch.start();
    {
        std::stringstream ss;
        bool has_items = first ? true : false;
        ss << "DisplayTimeEntryList open=" << open
           << ", has items=" << has_items;
        logger().debug(ss.str());
    }
    on_display_time_entry_list_(open, first);
    stopwatch.stop();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList done in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

void GUI::DisplayTags(std::vector<std::string> *tags) {
    logger().debug("DisplayTags");

    TogglGenericView *first = 0;
    for (std::vector<std::string>::const_iterator it = tags->begin();
            it != tags->end(); it++) {
        std::string name = *it;
        TogglGenericView *item = tag_to_view_item(name);
        item->Next = first;
        first = item;
    }
    on_display_tags_(first);
    view_item_clear(first);
}

void GUI::DisplayClientSelect(std::vector<toggl::Client *> *clients) {
    logger().debug("DisplayClientSelect");

    TogglGenericView *first = 0;
    for (std::vector<toggl::Client *>::const_iterator it = clients->begin();
            it != clients->end(); it++) {
        TogglGenericView *item = client_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_client_select_(first);
    view_item_clear(first);
}

void GUI::DisplayWorkspaceSelect(std::vector<toggl::Workspace *> *list) {
    logger().debug("DisplayWorkspaceSelect");

    TogglGenericView *first = 0;
    for (std::vector<toggl::Workspace *>::const_iterator it =
        list->begin(); it != list->end(); it++) {
        TogglGenericView *item = workspace_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_workspace_select_(first);
    view_item_clear(first);
}

void GUI::DisplayTimeEntryEditor(const _Bool open,
                                 TogglTimeEntryView *te,
                                 const std::string focused_field_name) {
    logger().debug("DisplayTimeEntryEditor");
    char_t *field_s = copy_string(focused_field_name);
    on_display_time_entry_editor_(open, te, field_s);
    free(field_s);
}

void GUI::DisplayURL(const std::string URL) {
    logger().debug("DisplayURL");

    char_t *url_s = copy_string(URL);
    on_display_url_(url_s);
    free(url_s);
}

void GUI::DisplaySettings(const _Bool open,
                          const _Bool record_timeline,
                          const Settings settings,
                          const _Bool use_proxy,
                          const Proxy proxy) {
    logger().debug("DisplaySettings");

    TogglSettingsView *view = settings_view_item_init(
        record_timeline,
        settings,
        use_proxy,
        proxy);

    on_display_settings_(open, view);

    settings_view_item_clear(view);
}

void GUI::DisplayTimerState(TogglTimeEntryView *te) {
    logger().debug("DisplayTimerState");
    on_display_timer_state_(te);
}

void GUI::DisplayIdleNotification(const std::string guid,
                                  const std::string since,
                                  const std::string duration,
                                  const uint64_t started) {
    char_t *guid_s = copy_string(guid);
    char_t *since_s = copy_string(since);
    char_t *duration_s = copy_string(duration);
    on_display_idle_notification_(guid_s,
                                  since_s,
                                  duration_s,
                                  started);
    free(guid_s);
    free(since_s);
    free(duration_s);
}

_Bool GUI::isNetworkingError(const error err) const {
    std::string value(err);
    if (value.find("Cannot establish proxy connection") != std::string::npos) {
        return true;
    }
    if (value.find("certificate verify failed") != std::string::npos) {
        return true;
    }
    if (value.find("Proxy Authentication Required") != std::string::npos) {
        return true;
    }
    if (value.find("Cannot assign requested address") != std::string::npos) {
        return true;
    }
    if (value.find("Certificate validation error") != std::string::npos) {
        return true;
    }
    if (value.find("Unacceptable certificate from www.toggl.com")
            != std::string::npos) {
        return true;
    }
    if (value.find("Host not found") != std::string::npos) {
        return true;
    }
    if (value.find("Cannot upgrade to WebSocket connection")
            != std::string::npos) { // NOLINT
        return true;
    }
    if (value.find("No message received") != std::string::npos) {
        return true;
    }
    if (value.find("Connection refused") != std::string::npos) {
        return true;
    }
    if (value.find("Connection timed out") != std::string::npos) {
        return true;
    }
    if (value.find("connect timed out") != std::string::npos) {
        return true;
    }
    if (value.find("SSL connection unexpectedly closed") != std::string::npos) {
        return true;
    }
    if (value.find("Network is down") != std::string::npos) {
        return true;
    }
    if (value.find("Network is unreachable") != std::string::npos) {
        return true;
    }
    if (value.find("Host is down") != std::string::npos) {
        return true;
    }
    if (value.find("No route to host") != std::string::npos) {
        return true;
    }
    if ((value.find("I/O error: 1") != std::string::npos)
            && (value.find(":443") != std::string::npos)) {
        return true;
    }
    if (value.find("The request timed out") != std::string::npos) {
        return true;
    }
    if (value.find("Could not connect to the server") != std::string::npos) {
        return true;
    }
    if (value.find("Connection reset by peer") != std::string::npos) {
        return true;
    }
    if (value.find("The Internet connection appears to be offline")
            != std::string::npos) {
        return true;
    }
    if (value.find("Timeout") != std::string::npos) {
        return true;
    }
    if (value.find("SSL Exception") != std::string::npos) {
        return true;
    }
    return false;
}

_Bool GUI::isUserError(const error err) const {
    if (noError == err) {
        return false;
    }
    std::string value(err);
    if (value.find("is suspended") != std::string::npos) {
        return true;
    }
    if (value.find("Request to server failed with status code: 403")
            != std::string::npos) {
        return true;
    }
    if (value.find("This version of the app is not supported")
            != std::string::npos) {
        return true;
    }
    if (value.find("Stop time must be after start time")
            != std::string::npos) {
        return true;
    }
    if (value.find("Invalid e-mail or password") != std::string::npos) {
        return true;
    }
    if (value.find("Maximum length for description") != std::string::npos) {
        return true;
    }
    if (value.find("Start time year must be between 2010 and 2100")
            != std::string::npos) {
        return true;
    }
    if (value.find("Missing workspace ID") != std::string::npos) {
        return true;
    }
    if (value.find(kEndpointGoneError) != std::string::npos) {
        return true;
    }
    return false;
}

}  // namespace toggl
