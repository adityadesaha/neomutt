/**
 * @file
 * Shared code for the Alias and Query Dialogs
 *
 * @authors
 * Copyright (C) 2020 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @page alias_gui Shared code for the Alias and Query Dialogs
 *
 * Shared code for the Alias and Query Dialogs
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "mutt/lib.h"
#include "address/lib.h"
#include "config/lib.h"
#include "gui.h"
#include "lib.h"
#include "alias.h"

#define RSORT(num) ((C_SortAlias & SORT_REVERSE) ? -num : num)

/**
 * alias_sort_name - Compare two Aliases by their short names
 * @param a First  Alias to compare
 * @param b Second Alias to compare
 * @retval -1 a precedes b
 * @retval  0 a and b are identical
 * @retval  1 b precedes a
 */
int alias_sort_name(const void *a, const void *b)
{
  const struct AliasView *av_a = a;
  const struct AliasView *av_b = b;

  int r = mutt_istr_cmp(av_a->alias->name, av_b->alias->name);

  return RSORT(r);
}

/**
 * alias_sort_address - Compare two Aliases by their Addresses
 * @param a First  Alias to compare
 * @param b Second Alias to compare
 * @retval -1 a precedes b
 * @retval  0 a and b are identical
 * @retval  1 b precedes a
 */
int alias_sort_address(const void *a, const void *b)
{
  const struct AddressList *al_a =
      &((struct AliasView const *) a)->alias->addr;
  const struct AddressList *al_b =
      &((struct AliasView const *) b)->alias->addr;

  int r;
  if (al_a == al_b)
    r = 0;
  else if (!al_a)
    r = -1;
  else if (!al_b)
    r = 1;
  else
  {
    const struct Address *addr_a = TAILQ_FIRST(al_a);
    const struct Address *addr_b = TAILQ_FIRST(al_b);
    if (addr_a && addr_a->personal)
    {
      if (addr_b && addr_b->personal)
        r = mutt_istr_cmp(addr_a->personal, addr_b->personal);
      else
        r = 1;
    }
    else if (addr_b && addr_b->personal)
      r = -1;
    else if (addr_a && addr_b)
      r = mutt_istr_cmp(addr_a->mailbox, addr_b->mailbox);
    else
      r = 0;
  }
  return RSORT(r);
}

/**
 * menu_data_alias_add - Add an Alias to the AliasMenuData
 * @param mdata Menu data holding Aliases
 * @param alias Alias to add
 *
 * @note The Alias is wrapped in an AliasView
 * @note Call menu_data_sort() to sort and reindex the AliasMenuData
 */
int menu_data_alias_add(struct AliasMenuData *mdata, struct Alias *alias)
{
  if (!mdata || !alias)
    return -1;

  struct AliasView av = {
    .num = 0,
    .is_tagged = false,
    .is_deleted = false,
    .alias = alias
  };
  ARRAY_ADD(mdata, av);
  return ARRAY_SIZE(mdata);
}

/**
 * menu_data_alias_delete - Delete an Alias from the AliasMenuData
 * @param mdata Menu data holding Aliases
 * @param alias Alias to remove
 *
 * @note Call menu_data_sort() to sort and reindex the AliasMenuData
 */
int menu_data_alias_delete(struct AliasMenuData *mdata, struct Alias *alias)
{
  if (!mdata || !alias)
    return -1;

  struct AliasView *avp = NULL;
  ARRAY_FOREACH(avp, mdata)
  {
    if (avp->alias != alias)
      continue;

    ARRAY_REMOVE(mdata, avp);
    break;
  }

  return ARRAY_SIZE(mdata);
}

/**
 * menu_data_sort - Sort and reindex an AliasMenuData
 * @param mdata Menu data holding Aliases
 */
void menu_data_sort(struct AliasMenuData *mdata)
{
  if ((C_SortAlias & SORT_MASK) != SORT_ORDER)
  {
    ARRAY_SORT(mdata, ((C_SortAlias & SORT_MASK) == SORT_ADDRESS) ? alias_sort_address : alias_sort_name);
  }

  struct AliasView *avp = NULL;
  int i = 0;
  ARRAY_FOREACH(avp, mdata)
  {
    avp->num = i++;
  }
}
