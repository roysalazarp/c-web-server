# c-web-server

-- RENDER PAGE --

html is the markup file loaded into a char*

char* country[2] = { "v0", "" };
render_val(country[0], country[1], html);

char* phone_number[2] = { "v1", "" };
render_val(phone_number[0], phone_number[1], html);

char* city_location[2] = { "v2", "" };
render_val(city_location[0], city_location[1], html);

char* guests_person_name[9] = { "for0", "for0->v0", "", "", "", "", "", "", "" };
render_for(guests_person_name[0], guests_person_name[1], html);

char* guests_person_age[9] = { "for0", "for0->v1", "", "", "", "", "", "", "" };
render_for(guests_person_age[0], guests_person_age[1], guests_person_age[2], html);

char* is_available_time = { "if0->v0", "" };
render_if(1, is_available_time[0], is_available_time[1], html);

char* employees_person_name[5] = { "for2", "for2->v0", "", "", "" };
render_for(guests_person_age[0], guests_person_age[1], guests_person_age[2], html);

char* employees_person_age[5] = { "for2", "for2->v1", "", "", "" };
render_for(employees_person_age[0], employees_person_age[1], employees_person_age[2], html);

char* employees_identity_contact[4] = { "for2->for1", "for2->for1->v0", "", "" };
render_for(employees_identity_contact[0], employees_identity_contact[1], employees_identity_contact[2], html);

char* employees_identity_fingerprint[4] = { "for2->for1", "for2->for1->v1", "", "" };
render_for(employees_identity_fingerprint[0], employees_identity_fingerprint[1], employees_identity_fingerprint[2], html);

char* invitees_belongings[6] = { "for1", "for1->v0", "", "", "", "" };
render_for(invitees_belongings[0], invitees_belongings[1], invitees_belongings[2], html);

char* invitees_address[6] = { "for1", "for1->v1", "", "", "", "" };
render_for(invitees_address[0], invitees_address[1], invitees_address[2], html);

-- RENDER COMPONENT --

char* component_html[2048];

// Same process than above for rendering ...
// 
// char* family_name[2] = { "v1", "" };
// render_val(family_name[0], family_name[1], component_html);
// 
// ...

char* menu_component[2] = { "c0", "rendered html...", html };
render_component(menu_component[0], menu_component[1], menu_component[2], html);



<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Document</title>
  </head>
  <body>
    <div>
        {{ v0 country }}
    </div>
    <div>{{ v2 phone_number }}</div>
    <span>{{ v1 city_location }}</span>
    
    {{ c0 menu_component }}
    
    {{ for0 guests }}
    <div>
        <div>{{ for0->v0 person_name }}</div>
        <div>
            <div>
                {{ for0->v1 person_age }}
            </div>
        </div>
    </div>
    {{ end for0 }}
    
    {{ if0 is_available }}
    <div>
        {{ if0->v0 time }}
    </div>
    {{ end if0 }}
    
    {{ for2 employees }}
    <div>
        <div>{{ for2->v0 person_name }}</div>
        <div>
            <div>
                {{ for2->v1 person_age }}
            </div>
            {{ for2->for1 identity }}
            <div>
                <div>{{ for2->for1->v0 contact }}</div>
                <div>
                    <div>
                        {{ for2->for1->v1 fingerprint }}
                    </div>
                    {{ c1 dropdown_component }}
                </div>
            </div>
            {{ end for2->for1 }}
        </div>
    </div>
    {{ end for2 }}
    
    {{ for1 invitees }}
    <div>
        <div>{{ for1->v0 belongings }}</div>
        <div>
            <div>
                {{ for1->v1 address }}
            </div>
        </div>
    </div>
    {{ end for1 }}


  </body>
</html>